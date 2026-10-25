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
	printf("===== MANDATORY =====\n");
	printf("--- %%c\n");
	T("%c", 'a');
	T("%c%c%c", 'a', 'b', 'c');
	T("[%c]", 0);
	T("%c", 255);
	T("%c", '\n');
	printf("--- %%s\n");
	T("%s", "hola");
	T("%s", "");
	T("%s", (char *)NULL);
	T("%s y %s", "uno", "dos");
	T("antes %s despues", "medio");
	printf("--- %%p\n");
	T("%p", &x);
	T("%p", (void *)0);
	T("%p", (void *)1);
	T("%p", (void *)LONG_MAX);
	T("%p", (void *)LONG_MIN);
	T("%p", (void *)ULONG_MAX);
	T("%p %p", &x, &big);
	printf("--- %%d %%i\n");
	T("%d", 0);
	T("%d", 42);
	T("%d", -42);
	T("%d", INT_MAX);
	T("%d", INT_MIN);
	T("%i", 7);
	T("%i", INT_MIN);
	T("%d %d %d", 1, -2, 3);
	printf("--- %%u\n");
	T("%u", 0u);
	T("%u", 42u);
	T("%u", UINT_MAX);
	T("%u", (unsigned int)-1);
	printf("--- %%x %%X\n");
	T("%x", 0);
	T("%x", 255);
	T("%X", 255);
	T("%x", UINT_MAX);
	T("%X", UINT_MAX);
	T("%x", 48879);
	T("%X", 0xABCDEF);
	printf("--- %%%%\n");
	T("%%");
	T("%%%%");
	T("100%%");
	T("%%c %%s %%d");
	T("%c%%%c", 'a', 'b');
	printf("--- mezclas\n");
	T("%c %s %p %d %i %u %x %X %%", 'a', "hi", &x, -1, 2, 3u, 255, 255);
	T("texto sin formato");
	T("");
	T("\n");
	T("a%sb%dc%xd", "S", 5, 255);
	printf("--- buffer (BUFFER_SIZE = 1024)\n");
	T("%s", mid);
	T("%s", big);
	T("hola %s", big);
	T("%s|%s", mid, mid);
	T("%s%s%s", mid, mid, mid);
	T("%d%s%d", 5, mid, 6);
	T("%s%c", mid, 'z');
	T("%c%s%c", 'a', mid, 'z');
	printf("\n%s%d OK%s / %s%d KO%s\n", GREEN, g_ok, RESET, RED, g_ko, RESET);
	return (g_ko != 0);
}
