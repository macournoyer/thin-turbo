#include "tmpfile.h"

/* event callbacks */

static void tmpfile_writable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  
}


static void tmpfile_readable_cb(EV_P_ struct ev_io *watcher, int revents)
{
  
}

void tmpfile_watch(buffer_t *buf)
{
  buf->fname = strdup(TMPFILE_TEMPLATE);
  buf->fd    = mkstemp(buf->fname);

  if (buf->fd < 0) {
    /* error, cancel watching */
    buf->fd = 0;
    free(buf->fname);
    buf->fname = NULL;
    return;
  }
  
  ev_io_init(&conn->event##_watcher, cb, conn->fd, ev_event); \
  conn->event##_watcher.data = conn; \
  ev_io_start(conn->loop, &conn->event##_watcher);
}

void tmpfile_close(buffer_t *buf)
{
  if (buf->fd) {
    close(buf->fd);
    unlink(buf->fname);
    free(buf->fname);
  }
}
