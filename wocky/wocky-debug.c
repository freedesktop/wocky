
#include <stdarg.h>

#include <glib.h>

#include "wocky-debug.h"

static DebugFlags flags = 0;
static gboolean initialized = FALSE;

static GDebugKey keys[] = {
  { "net",            DEBUG_NET },
  { "xmpp",           DEBUG_XMPP },
  { "xmpp-reader",    DEBUG_XMPP_READER },
  { "xmpp-writer",    DEBUG_XMPP_WRITER },
  { "all",            ~0 },
  { 0, },
};

void wocky_debug_set_flags_from_env ()
{
  guint nkeys;
  const gchar *flags_string;

  for (nkeys = 0; keys[nkeys].value; nkeys++);

  flags_string = g_getenv ("WOCKY_DEBUG");

  if (flags_string)
    wocky_debug_set_flags (g_parse_debug_string (flags_string, keys, nkeys));

  initialized = TRUE;
}

void wocky_debug_set_flags (DebugFlags new_flags)
{
  flags |= new_flags;
  initialized = TRUE;
}

gboolean wocky_debug_flag_is_set (DebugFlags flag)
{
  return flag & flags;
}

void wocky_debug (DebugFlags flag,
                   const gchar *format,
                   ...)
{
  if (G_UNLIKELY(!initialized))
    wocky_debug_set_flags_from_env();
  if (flag & flags)
    {
      va_list args;
      va_start (args, format);
      g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, format, args);
      va_end (args);
    }
}