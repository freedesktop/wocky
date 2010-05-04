/* wocky-auth-registry.h */

#ifndef _WOCKY_AUTH_REGISTRY_H
#define _WOCKY_AUTH_REGISTRY_H

#include <glib-object.h>
#include <gio/gio.h>
#include "wocky-sasl-handler.h"

G_BEGIN_DECLS

GQuark wocky_auth_error_quark (void);
#define WOCKY_AUTH_ERROR \
  wocky_auth_error_quark ()

typedef enum
{
  /* Failed to initialize our sasl support */
  WOCKY_AUTH_ERROR_INIT_FAILED,
  /* Server doesn't support sasl (no mechanisms) */
  WOCKY_AUTH_ERROR_SASL_NOT_SUPPORTED,
  /* Server doesn't support any mechanisms that we support */
  WOCKY_AUTH_ERROR_NO_SUPPORTED_MECHANISMS,
  /* Couldn't send our stanzas to the server */
  WOCKY_AUTH_ERROR_NETWORK,
  /* Server sent an invalid reply */
  WOCKY_AUTH_ERROR_INVALID_REPLY,
  /* Failure to provide user credentials */
  WOCKY_AUTH_ERROR_NO_CREDENTIALS,
  /* Server sent a failure */
  WOCKY_AUTH_ERROR_FAILURE,
  /* disconnected */
  WOCKY_AUTH_ERROR_CONNRESET,
  /* XMPP stream error while authing */
  WOCKY_AUTH_ERROR_STREAM,
} WockyAuthError;

#define WOCKY_TYPE_AUTH_REGISTRY wocky_auth_registry_get_type()

#define WOCKY_AUTH_REGISTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  WOCKY_TYPE_AUTH_REGISTRY, WockyAuthRegistry))

#define WOCKY_AUTH_REGISTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  WOCKY_TYPE_AUTH_REGISTRY, WockyAuthRegistryClass))

#define WOCKY_IS_AUTH_REGISTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  WOCKY_TYPE_AUTH_REGISTRY))

#define WOCKY_IS_AUTH_REGISTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  WOCKY_TYPE_AUTH_REGISTRY))

#define WOCKY_AUTH_REGISTRY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  WOCKY_TYPE_AUTH_REGISTRY, WockyAuthRegistryClass))

typedef struct _WockyAuthRegistry WockyAuthRegistry;
typedef struct _WockyAuthRegistryClass WockyAuthRegistryClass;
typedef struct _WockyAuthRegistryPrivate WockyAuthRegistryPrivate;

struct _WockyAuthRegistry
{
  GObject parent;

  WockyAuthRegistryPrivate *priv;
};

struct _WockyAuthRegistryClass
{
  GObjectClass parent_class;
};

GType wocky_auth_registry_get_type (void) G_GNUC_CONST;

WockyAuthRegistry *wocky_auth_registry_new (void);

void wocky_auth_registry_start_auth_async (WockyAuthRegistry *self,
    const GSList *mechanisms,
    gboolean allow_plain,
    gboolean is_secure_channel,
    const gchar *username,
    const gchar *password,
    const gchar *server,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean wocky_auth_registry_start_auth_finish (WockyAuthRegistry *self,
    GAsyncResult *res,
    gchar **mechanism,
    GString **initial_response,
    GError **error);

void wocky_auth_registry_challenge_async (WockyAuthRegistry *self,
    const GString *challenge_data,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean wocky_auth_registry_challenge_finish (WockyAuthRegistry *self,
    GAsyncResult *res,
    GString **response,
    GError **error);

void wocky_auth_registry_success_async (WockyAuthRegistry *self,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean wocky_auth_registry_success_finish (WockyAuthRegistry *self,
    GAsyncResult *res,
    GError **error);

void wocky_auth_registry_add_handler (WockyAuthRegistry *self,
    WockySaslHandler *handler);

G_END_DECLS

#endif /* _WOCKY_AUTH_REGISTRY_H */