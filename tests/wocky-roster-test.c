#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <wocky/wocky-roster.h>
#include <wocky/wocky-porter.h>
#include <wocky/wocky-utils.h>
#include <wocky/wocky-xmpp-connection.h>

#include "wocky-test-stream.h"
#include "wocky-test-helper.h"

static void
test_instantiation (void)
{
  WockyRoster *roster;
  WockyXmppConnection *connection;
  WockyPorter *porter;
  WockyTestStream *stream;

  stream = g_object_new (WOCKY_TYPE_TEST_STREAM, NULL);
  connection = wocky_xmpp_connection_new (stream->stream0);
  porter = wocky_porter_new (connection);

  roster = wocky_roster_new (connection, porter);

  g_assert (roster != NULL);

  g_object_unref (roster);
  g_object_unref (porter);
  g_object_unref (connection);
  g_object_unref (stream);
}

static gboolean
fetch_roster_cb (WockyPorter *porter,
    WockyXmppStanza *stanza,
    gpointer user_data)
{
  test_data_t *test = (test_data_t *) user_data;
  WockyStanzaType type;
  WockyStanzaSubType sub_type;
  WockyXmppNode *node;

  /* Make sure stanza is as expected. */
  wocky_xmpp_stanza_get_type_info (stanza, &type, &sub_type);

  g_assert (type == WOCKY_STANZA_TYPE_IQ);
  g_assert (sub_type == WOCKY_STANZA_SUB_TYPE_GET);

  node = wocky_xmpp_node_get_child (stanza->node, "query");

  g_assert (stanza->node != NULL);
  g_assert (!wocky_strdiff (wocky_xmpp_node_get_ns (node),
          "jabber:iq:roster"));

  test->outstanding--;
  g_main_loop_quit (test->loop);
  return TRUE;
}

static void
test_fetch_roster (void)
{
  WockyRoster *roster;
  test_data_t *test = setup_test ();

  test_open_both_connections (test);

  wocky_porter_register_handler (test->sched_out,
      WOCKY_STANZA_TYPE_IQ, WOCKY_STANZA_SUB_TYPE_GET, NULL,
      WOCKY_PORTER_HANDLER_PRIORITY_MAX,
      fetch_roster_cb, test, WOCKY_STANZA_END);

  wocky_porter_start (test->sched_out);
  wocky_porter_start (test->sched_in);

  roster = wocky_roster_new (test->in, test->sched_in);

  wocky_roster_fetch_roster (roster);
  test->outstanding++;

  test_wait_pending (test);

  test_close_both_porters (test);
  teardown_test (test);
}

int
main (int argc, char **argv)
{
  g_thread_init (NULL);

  g_test_init (&argc, &argv, NULL);
  g_type_init ();

  g_test_add_func ("/xmpp-roster/instantiation", test_instantiation);
  g_test_add_func ("/xmpp-roster/fetch-roster", test_fetch_roster);

  return g_test_run ();
}