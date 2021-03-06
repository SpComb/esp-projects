#include "cli.h"
#include <cli.h>
#include <logging.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static void cli_print_ctx(const struct cmdctx *ctx)
{
  if (ctx->parent) {
    cli_print_ctx(ctx->parent);
  }

  if (ctx->cmd) {
    printf(" %s", ctx->cmd->name);
  }
}

static int cli_print_help(const struct cmdtab *cmdtab, const struct cmdctx *parent)
{
  struct cmdctx ctx = {
    .parent = parent,
    .cmdtab = cmdtab,
  };

  for (const struct cmd *cmd = cmdtab->commands; cmd->name; cmd++) {
    ctx.cmd = cmd;

    if (cmd->func) {
      cli_print_ctx(&ctx);

      if (cmd->usage) {
        printf(" %s", cmd->usage);
      }

      if (cmd->describe) {
        printf(": %s\n", cmd->describe);
      } else {
        printf("\n");
      }
    }

    if (cmd->subcommands) {
      cli_print_help(cmd->subcommands, &ctx);
    }
  }

  return 0;
}

static int cli_cmd_error(const struct cmdctx *ctx, enum cmd_error err, const char *arg)
{
  printf("!");
  if (ctx->cmd || ctx->parent) {
    cli_print_ctx(ctx);
    printf(": ");
  } else {
    printf(" ");
  }
  printf("%s", cmd_strerror(err));
  if ((err & CMD_ERR_USAGE) && ctx->cmd && ctx->cmd->usage)
    printf(": %s\n", ctx->cmd->usage);
  else if (arg)
    printf(": %s\n", arg);
  else
    printf("\n");

  if (err == CMD_ERR_NOT_FOUND || err == CMD_ERR_MISSING_SUBCOMMAND) {
    cli_print_help(ctx->cmdtab, ctx->parent);
  }

  return 0;
}

// process one line of input
static int cli_read(struct cli *cli)
{
  char *ptr = cli->buf;
  int c;

  printf("> ");

  while ((c = fgetc(stdin)) != EOF) {
    #ifdef DEBUG
      if (isprint(c)) {
        LOG_DEBUG("%c", c);
      } else {
        LOG_DEBUG("%#02x", c);
      }
    #endif

    if (ptr >= cli->buf + cli->size) {
      LOG_WARN("line overflow");
      return 1;
    }

    // echo
    fputc(c, stdout);

    if (c == '\b' && ptr > cli->buf) {
      // erase one char
      ptr--;

      // echo wipeout
      fprintf(stdout, " \b");

    } else if (c == '\r') {
      // skip
      continue;

    } else if (c == '\n') {
      *ptr = '\0';

      return 0;
    } else {
      // copy
      *ptr++ = c;
    }
  }

  LOG_WARN("EOF");
  return 1;
}

static int cli_eval(struct cli *cli)
{
  const char *line = cli->buf;
  int err;

  if ((err = cmd_eval(&cli->cmdtab, line)) < 0) {
    LOG_ERROR("cmd_eval %s: %s", line, cmd_strerror(-err));
  } else if (err) {
    LOG_ERROR("cmd %s: %d", line, err);
  }

  return err;
}

void cli_main(struct cli *cli)
{
  int err;

  LOG_DEBUG("cli=%p", cli);

  for (;;) {
    // read lines of input, ignore errors
    if ((err = cli_read(cli))) {
      LOG_WARN("cli_read");
      return err;
    }

    // evaluatate
    if ((err = cli_eval(cli))) {
      return err;
    }
  }
}

int cli_init(struct cli **clip, const struct cmd *commands, size_t buf_size)
{
  struct cli *cli;
  int err = 0;

  LOG_INFO("cli buf_size=%u", buf_size);

  if (!(cli = calloc(1, sizeof(*cli)))) {
    LOG_ERROR("calloc");
    return -1;
  }

  if (!(cli->buf = malloc(buf_size))) {
    LOG_ERROR("malloc buf");
    goto error;
  }

  cli->size = buf_size;
  cli->cmdtab = (struct cmdtab) {
    .commands      = commands,
    .error_handler = cli_cmd_error,
  };

  *clip = cli;

  return 0;

error:
  if (cli->buf) {
    free(cli->buf);
  }

  free(cli);

  return err;
}

int cmd_help(struct cli *cli)
{
  return cli_print_help(&cli->cmdtab, NULL);
}
