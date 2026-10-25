#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

int	ft_printf(const char *format, ...);

#define BUF 16384
#define GREEN "\033[32m"
#define RED "\033[31m"
#define RESET "\033[0m"

static int	g_ok;
static int	g_ko;

static int	slurp(int fd, char *buf)
{
	int	n;
	int	r;

	n = 0;
	r = read(fd, buf, BUF - 1 - n);
	while (r > 0)
	{
		n += r;
		r = read(fd, buf + n, BUF - 1 - n);
	}
	buf[n] = '\0';
	return (n);
}

static void	report(const char *call, const char *exp, int er,
		const char *got, int gr)
{
	if (!strcmp(exp, got) && er == gr)
	{
		g_ok++;
		printf(GREEN "[OK]" RESET " %.70s\n", call);
		return ;
	}
	g_ko++;
	printf(RED "[KO]" RESET " %.70s\n", call);
	printf("     printf   (%d): [%.60s]%s\n", er, exp,
		strlen(exp) > 60 ? "..." : "");
	printf("     ft_printf(%d): [%.60s]%s\n", gr, got,
		strlen(got) > 60 ? "..." : "");
}

/* Ejecuta printf y ft_printf con los mismos argumentos y compara salida y retorno */
#define T(...) \
	do { \
		char	exp[BUF]; \
		char	got[BUF]; \
		int		er; \
		int		gr; \
		int		out[2]; \
		int		ret[2]; \
		er = snprintf(exp, BUF, __VA_ARGS__); \
		gr = -99; \
		if (pipe(out) || pipe(ret)) \
			return (1); \
		fflush(stdout); \
		if (fork() == 0) \
		{ \
			dup2(out[1], 1); \
			close(out[0]); \
			gr = ft_printf(__VA_ARGS__); \
			if (write(ret[1], &gr, sizeof(gr)) < 0) \
				_exit(1); \
			_exit(0); \
		} \
		close(out[1]); \
		close(ret[1]); \
		slurp(out[0], got); \
		if (read(ret[0], &gr, sizeof(gr)) <= 0) \
			gr = -99; \
		wait(NULL); \
		close(out[0]); \
		close(ret[0]); \
		report(#__VA_ARGS__, exp, er, got, gr); \
	} while (0)

int	main(void)
{
	char	big[3000];
	char	mid[1100];
	int		x;

	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	memset(mid, 'b', sizeof(mid) - 1);
	mid[sizeof(mid) - 1] = '\0';
	printf("===== BONUS =====\n");
	printf("--- conversiones basicas\n");
	T("%c|%s|%p|%d|%i|%u|%x|%X|%%", 'a', "hi", &x, -1, 2, 3u, 255, 255);
	T("%s|%p", (char *)NULL, (void *)0);
	T("%d|%u|%x", INT_MIN, UINT_MAX, UINT_MAX);
	printf("--- %%c con ancho y -\n");
	T("%5c|%-5c|", 'a', 'b');
	T("%1c|%-1c|", 'a', 'b');
	T("%3c|", 0);
	printf("--- %%s con ancho y precision\n");
	T("%10s|%-10s|", "abc", "abc");
	T("%2s|%-2s|", "abcdef", "abcdef");
	T("%.3s|%.0s|%.10s|", "abcdef", "abcdef", "abcdef");
	T("%8.3s|%-8.3s|", "abcdef", "abcdef");
	T("%.s|%5.s|", "abcdef", "abcdef");
	T("%s|%10s|%.3s|%.6s|%.10s|", (char *)NULL, (char *)NULL, (char *)NULL,
		(char *)NULL, (char *)NULL);
	T("%5s|%-5s|", "", "");
	printf("--- %%p con ancho y -\n");
	T("%20p|%-20p|", &x, &x);
	T("%5p|%-5p|", (void *)0, (void *)0);
	T("%p|%20p", (void *)ULONG_MAX, (void *)LONG_MIN);
	printf("--- %%d %%i: ancho, -, 0\n");
	T("%5d|%-5d|%05d|%-05d|%0-5d|", 42, 42, 42, 42, 42);
	T("%5d|%-5d|%05d|", -42, -42, -42);
	T("%2d|%-2d|%02d|", 12345, 12345, 12345);
	T("%10d|%010d|", INT_MIN, INT_MIN);
	T("%10i|%-10i|%010i|", 99, 99, 99);
	printf("--- %%d %%i: + y espacio\n");
	T("%+d|% d|%+d|% d|", 42, 42, -42, -42);
	T("%+d|% d|%+d|", 0, 0, INT_MAX);
	T("% d|%+d|", INT_MIN, INT_MIN);
	T("%+5d|% 5d|%+-5d|% -5d|%+05d|% 05d|", 42, 42, 42, 42, 42, 42);
	T("%+ d|% +d|", 5, 5);
	printf("--- %%d %%i: precision\n");
	T("%.5d|%.1d|%.2d|%.5d|", 42, 42, 12345, -42);
	T("%8.5d|%-8.5d|%08.5d|", 42, 42, 42);
	T("%8.5d|%-8.5d|%+8.5d|% 8.5d|", -42, -42, 42, 42);
	T("%.12d|%.3d|", INT_MIN, INT_MAX);
	T("%+.3d|% .3d|", 7, 7);
	printf("--- precision 0 con valor 0 (no imprime nada)\n");
	T("%.0d|%.d|%5.0d|%-5.0d|", 0, 0, 0, 0);
	T("%+.0d|% .0d|", 0, 0);
	T("%.0d|%.0i|", 1, 10);
	T("%.0u|%.0x|%.0X|", 0u, 0u, 0u);
	T("%5.0u|%5.0x|", 0u, 0u);
	T("%#.0x|%#5.0x|", 0u, 0u);
	printf("--- %%u\n");
	T("%u|%10u|%-10u|%010u|%.8u|", 42u, 42u, 42u, 42u, 42u);
	T("%+u|% u|", 5u, 5u);
	T("%u|%.0u|%5.3u|", 0u, 7u, 7u);
	printf("--- %%x %%X\n");
	T("%x|%10x|%-10x|%010x|%.8x|", 48879u, 48879u, 48879u, 48879u, 48879u);
	T("%X|%10X|%-10X|%010X|%.8X|", 48879u, 48879u, 48879u, 48879u, 48879u);
	T("%x|%X|", 0u, 0u);
	printf("--- # (prefijo 0x)\n");
	T("%#x|%#X|%#x|%#X|", 255u, 255u, 0u, 0u);
	T("%#8x|%-#8x|%#08x|%#08X|", 255u, 255u, 255u, 255u);
	T("%#.5x|%#10.5x|%#-10.5x|", 255u, 255u, 255u);
	T("%#x|%#10x|%#010x|", UINT_MAX, UINT_MAX, UINT_MAX);
	T("%#2x|%#-2x|", 4096u, 4096u);
	printf("--- %%%% con modificadores\n");
	T("%5%|%-5%|%05%|");
	printf("--- mezclas\n");
	T("[%-5d][%5s][%#x][%05.3d][% d]", 42, "ab", 255u, 7, 9);
	T("%c%5c%-5c%c|", 'a', 'b', 'c', 'd');
	T("%s %10s %-10s|", "a", "b", "c");
	T("sin formato");
	T("");
	T("%d%d%d", 1, 2, 3);
	printf("--- buffer (BUFFER_SIZE = 1024)\n");
	T("%s", mid);
	T("%s", big);
	T("hola %s", big);
	T("%s|%s", mid, mid);
	T("%s%s%s", mid, mid, mid);
	T("%d%s%d", 5, mid, 6);
	T("%1000d|%1200d|", 1, 2);
	T("%-1100d|", 7);
	T("%1100s|", "x");
	T("%1100c|", 'x');
	T("%2000s|", big);
	T("%.2000d|", 7);
	T("%2000.1500d|", 7);
	T("%#2000x|", 255u);
	printf("\n%s%d OK%s / %s%d KO%s\n", GREEN, g_ok, RESET, RED, g_ko, RESET);
	return (g_ko != 0);
}
