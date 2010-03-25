/*
 * wocky-pubsub-helpers.c — PubSub helper functions
 * Copyright © 2009–2010 Collabora Ltd.
 * Copyright © 2010 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "wocky-pubsub-helpers.h"

#include "wocky-namespaces.h"
#include "wocky-pubsub-service.h"

/**
 * wocky_pubsub_make_publish_stanza:
 * @service: the JID of a PubSub service, or %NULL
 * @node: the name of a node on @service; may not be %NULL
 * @pubsub_out: address at which to store a pointer to the <pubsub/> node
 * @publish_out: address at which to store a pointer to the <publish/> node
 * @item_out: address at which to store a pointer to the <item/> node
 *
 * <!-- -->
 *
 * Returns: a new iq[type='set']/pubsub/publish/item stanza
 */
WockyXmppStanza *
wocky_pubsub_make_publish_stanza (
    const gchar *service,
    const gchar *node,
    WockyXmppNode **pubsub_out,
    WockyXmppNode **publish_out,
    WockyXmppNode **item_out)
{
  WockyXmppNode *pubsub, *publish, *item;
  WockyXmppStanza *stanza;

  g_return_val_if_fail (node != NULL, NULL);

  stanza = wocky_xmpp_stanza_build (
      WOCKY_STANZA_TYPE_IQ, WOCKY_STANZA_SUB_TYPE_SET,
      NULL, service,
        WOCKY_NODE, "pubsub",
          WOCKY_NODE_XMLNS, WOCKY_XMPP_NS_PUBSUB,
          WOCKY_NODE_ASSIGN_TO, &pubsub,
          WOCKY_NODE, "publish",
            WOCKY_NODE_ASSIGN_TO, &publish,
            WOCKY_NODE_ATTRIBUTE, "node", node,
            WOCKY_NODE, "item",
              WOCKY_NODE_ASSIGN_TO, &item,
            WOCKY_NODE_END,
          WOCKY_NODE_END,
        WOCKY_NODE_END,
      WOCKY_STANZA_END);

  if (pubsub_out != NULL)
    *pubsub_out = pubsub;

  if (publish_out != NULL)
    *publish_out = publish;

  if (item_out != NULL)
    *item_out = item;

  return stanza;
}

static gboolean
get_pubsub_child_node (WockyXmppStanza *reply,
    const gchar *pubsub_ns,
    const gchar *child_name,
    WockyXmppNode **child_out,
    GError **error)
{
  WockyXmppNode *n;

  g_return_val_if_fail (reply != NULL, FALSE);

  n = wocky_xmpp_node_get_child_ns (reply->node, "pubsub", pubsub_ns);

  if (n == NULL)
    {
      g_set_error (error, WOCKY_PUBSUB_SERVICE_ERROR,
          WOCKY_PUBSUB_SERVICE_ERROR_WRONG_REPLY,
          "Reply doesn't contain <pubsub/> node");
      return FALSE;
    }

  n = wocky_xmpp_node_get_child (n, child_name);

  if (n == NULL)
    {
      g_set_error (error, WOCKY_PUBSUB_SERVICE_ERROR,
          WOCKY_PUBSUB_SERVICE_ERROR_WRONG_REPLY,
          "Reply doesn't contain <%s/> node", child_name);
      return FALSE;
    }

  if (child_out != NULL)
    *child_out = n;

  return TRUE;
}

static gboolean
wocky_pubsub_distill_iq_reply_internal (GObject *source,
    GAsyncResult *res,
    const gchar *pubsub_ns,
    const gchar *child_name,
    gboolean body_optional,
    WockyXmppNode **child_out,
    GError **error)
{
  WockyXmppStanza *reply;
  gboolean ret = FALSE;

  if (child_out != NULL)
    *child_out = NULL;

  /* Superlative news out of the 00:04 to Cambridge: earlier today, an
   * asynchronous method call announced its plans to bring a node to The
   * People's Republic of Wocky.
   */
  reply = wocky_porter_send_iq_finish (WOCKY_PORTER (source), res, error);

  if (reply == NULL)
    return FALSE;

  if (!wocky_xmpp_stanza_extract_errors (reply, NULL, error, NULL, NULL))
    {
      ret = TRUE;

      if (pubsub_ns != NULL)
        {
          /* A force of a thousand function calls will anchor the node to
           * a resplendent out parameter modeled on the Dear Leader's hand.
           */
          ret = get_pubsub_child_node (reply, pubsub_ns, child_name, child_out,
              error);

          /* The People's Great and Harmonious Node Pointer of Peter
           * Saint-Andre will conclude the most astonishing stanza breakdown
           * ever witnessed by man.
           */
          if (!ret && body_optional)
            {
              /* “The stanza is perfect. We have already succeeded.” */
              ret = TRUE;
              g_clear_error (error);
            }
        }
    }

  g_object_unref (reply);
  return ret;
}

/**
 * wocky_pubsub_distill_iq_reply:
 * @source: a #WockyPorter instance
 * @res: a result passed to the callback for wocky_porter_send_iq_async()
 * @pubsub_ns: the namespace of the <pubsub/> node expected in this reply (such
 *              WOCKY_XMPP_NS_PUBSUB), or %NULL if one is not expected
 * @child_name: the name of the child of <pubsub/> expected in this reply (such
 *              as "subscriptions"); ignored if @pubsub_ns is %NULL
 * @child_out: location at which to store a pointer to that child node, or
 *             %NULL if you don't need it
 * @error: location at which to store an error if the call to
 *         wocky_porter_send_iq_async() returned an error, or if the reply was
 *         an error
 *
 * Helper function to finish a wocky_porter_send_iq_async() operation
 * and extract a particular pubsub child from the resulting reply, if needed.
 *
 * Returns: %TRUE if the desired pubsub child was found; %FALSE if
 *          sending the IQ failed, the reply had type='error', or the
 *          pubsub child was not found, with @error set appropriately.
 */
gboolean
wocky_pubsub_distill_iq_reply (GObject *source,
    GAsyncResult *res,
    const gchar *pubsub_ns,
    const gchar *child_name,
    WockyXmppNode **child_out,
    GError **error)
{
  return wocky_pubsub_distill_iq_reply_internal (source, res, pubsub_ns,
      child_name, FALSE, child_out, error);
}

/**
 * wocky_pubsub_distill_void_iq_reply:
 * @source: a #WockyPorter instance
 * @res: a result passed to the callback for wocky_porter_send_iq_async()
 * @error: location at which to store an error if the call to
 *         wocky_porter_send_iq_async() returned an error, or if the reply was
 *         an error
 *
 * Helper function to finish a wocky_porter_send_iq_async() operation where no
 * pubsub child is expected in the resulting reply.
 *
 * Returns: %TRUE if the IQ was a success; %FALSE if
 *          sending the IQ failed or the reply had type='error',
 *          with @error set appropriately.
 */
gboolean
wocky_pubsub_distill_void_iq_reply (GObject *source,
    GAsyncResult *res,
    GError **error)
{
  return wocky_pubsub_distill_iq_reply_internal (source, res, NULL, NULL, TRUE,
      NULL, error);
}

/**
 * wocky_pubsub_distill_ambivalent_iq_reply:
 * @source: a #WockyPorter instance
 * @res: a result passed to the callback for wocky_porter_send_iq_async()
 * @pubsub_ns: the namespace of the <pubsub/> node accepted in this reply (such
 *              WOCKY_XMPP_NS_PUBSUB)
 * @child_name: the name of the child of <pubsub/> accepted in this reply (such
 *              as "subscriptions")
 * @child_out: location at which to store a pointer to the node named
 *             @child_name, if is found, or to be set to %NULL if it is not
 *             found
 * @error: location at which to store an error if the call to
 *         wocky_porter_send_iq_async() returned an error, or if the reply was
 *         an error
 *
 * Helper function to finish a wocky_porter_send_iq_async() operation
 * and extract a particular pubsub child from the resulting reply, if it is
 * present. This is like wocky_pubsub_distill_iq_reply(), but is ambivalent as
 * to whether the <pubsub/> structure has to be included.
 *
 * Returns: %TRUE if the IQ was a success; %FALSE if
 *          sending the IQ failed or the reply had type='error',
 *          with @error set appropriately.
 */
gboolean
wocky_pubsub_distill_ambivalent_iq_reply (GObject *source,
    GAsyncResult *res,
    const gchar *pubsub_ns,
    const gchar *child_name,
    WockyXmppNode **child_out,
    GError **error)
{
  return wocky_pubsub_distill_iq_reply_internal (source, res, pubsub_ns,
      child_name, TRUE, child_out, error);
}
