#include "vfs.h"

#include <logging.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/dirent.h>

static char dirent_type_char(const struct dirent *d) {
  if (d->d_type == DT_DIR) {
    return 'd';
  } else if (d->d_type == DT_REG) {
    return 'f';
  } else {
    return '?';
  }
}

int vfs_ls_cmd(int argc, char **argv, void *ctx)
{
  const char *path;
  DIR *dir;
  struct dirent *d;
  int err = 0;

  if ((err = cmd_arg_str(argc, argv, 1, &path))) {
    return err;
  }

  if (!(dir = opendir(path))) {
    LOG_ERROR("opendir %s: %s", path, strerror(errno));
    return -1;
  }

  // ignore any errors returned by readdir()
  // https://github.com/pellepl/spiffs/issues/64#issuecomment-171570238 returns an undocumented internal error code in normal case
  while ((d = readdir(dir))) {
    printf("%c %s\n", dirent_type_char(d), d->d_name);
  }

  closedir(dir);

  return err;
}

const struct cmd vfs_commands[] = {
  { "ls",     vfs_ls_cmd,    .usage = "PATH", .describe = "List files"  },
  {}
};

const struct cmdtab vfs_cmdtab = {
  .commands = vfs_commands,
};
