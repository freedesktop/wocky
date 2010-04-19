#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>

#include <wocky/wocky-node-tree.h>
#include <wocky/wocky-namespaces.h>

#include "wocky-test-helper.h"

static void
test_build_simple_tree (void)
{
  WockyNodeTree *tree;
  WockyNode *n;

  tree = wocky_node_tree_new ("lions", "animals",
    '(', "distribution",
      '$', "Only in kenya",
    ')',
    NULL);

  g_assert (tree != NULL);

  n = wocky_node_tree_get_top_node (tree);
  g_assert (n != NULL);
  g_assert_cmpstr (n->name, ==, "lions");
  g_assert_cmpstr (wocky_node_get_ns (n), ==, "animals");

  g_assert_cmpint (g_slist_length (n->children), ==, 1);
  n = wocky_node_get_first_child (n);
  g_assert (n != NULL);
  g_assert_cmpstr (n->name, ==, "distribution");
  g_assert_cmpstr (wocky_node_get_ns (n), ==, "animals");
  g_assert_cmpstr (n->content, ==, "Only in kenya");

  g_object_unref (tree);
}

int
main (int argc, char **argv)
{
  int result;

  test_init (argc, argv);

  g_test_add_func ("/xmpp-node-tree/simple-tree",
    test_build_simple_tree);

  result =  g_test_run ();
  test_deinit ();
  return result;
}
