#include <builtin.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

static void mode_str (mode_t mode, char* buf) {
	buf[0] = S_ISDIR (mode)	   ? 'd'
			 : S_ISCHR (mode)  ? 'c'
			 : S_ISBLK (mode)  ? 'b'
			 : S_ISFIFO (mode) ? 'p'
							   : '-';
	buf[1] = (mode & S_IRUSR) ? 'r' : '-';
	buf[2] = (mode & S_IWUSR) ? 'w' : '-';
	buf[3] = (mode & S_IXUSR) ? 'x' : '-';
	buf[4] = (mode & S_IRGRP) ? 'r' : '-';
	buf[5] = (mode & S_IWGRP) ? 'w' : '-';
	buf[6] = (mode & S_IXGRP) ? 'x' : '-';
	buf[7] = (mode & S_IROTH) ? 'r' : '-';
	buf[8] = (mode & S_IWOTH) ? 'w' : '-';
	buf[9] = (mode & S_IXOTH) ? 'x' : '-';
	buf[10] = 0;
}

static void fmt_time (time_t t, char* buf, size_t len) {
	struct tm* tm = localtime (&t);
	if (!tm) {
		snprintf (buf, len, "(unknown)");
		return;
	}
	strftime (buf, len, "%b %e %H:%M:%S %Y", tm);
}

static void print_stat (struct stat* st, const char* name) {
	char mode[11], at[32], mt[32], ct[32];
	mode_str (st->st_mode, mode);
	fmt_time (st->st_atime, at, sizeof at);
	fmt_time (st->st_mtime, mt, sizeof mt);
	fmt_time (st->st_ctime, ct, sizeof ct);
	printf ("%ld %ld %s %ld %d %d %ld %ld \"%s\" \"%s\" \"%s\" %ld %ld (%s)\n", (long)st->st_dev,
			(long)st->st_ino, mode, (long)st->st_nlink, (int)st->st_uid, (int)st->st_gid,
			(long)st->st_rdev, (long)st->st_size, at, mt, ct, (long)st->st_blksize,
			(long)st->st_blocks, name);
}

int builtin_stat (int argc, char** argv) {
	if (argc < 2) {
		struct stat st;
		if (fstat (0, &st) != 0) {
			perror ("stat");
			return 1;
		}
		print_stat (&st, "stdin");
		return 0;
	}

	int ret = 0;
	for (int i = 1; i < argc; i++) {
		struct stat st;
		if (stat (argv[i], &st) != 0) {
			perror (argv[i]);
			ret = 1;
			continue;
		}
		print_stat (&st, argv[i]);
	}
	return ret;
}