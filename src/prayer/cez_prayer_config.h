/* $Cambridge: hermes/src/prayer/shared/config.h,v 1.10 2012/06/30 14:30:08 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Prayer Configuration */

#define CONFIG_CWD_MAXLEN       (4096)

struct config_local_domain {    /* Local domain */
    struct list_item *next;     /* Linked list */
    char *name;                 /* Name of domain */
    char *file;                 /* CDB file assocaited with domain */
    void *cdb_map;              /* Open CDB map associated with domain */
};

struct config_http_port {       /* HTTP or HTTPS port */
    struct list_item *next;     /* Linked list */
    char *name;                 /* Name of port (unused: list placeholder) */
    unsigned long port;         /* Intenet domain port number */
    char *interface;            /* Interface name or IP (NIL => all) */
    BOOL use_ssl;               /* T => Connections here will use SSL */
};

struct config_language {        /* Ispell languages */
    struct list_item *next;     /* Linked list */
    char *name;                 /* Ispell name of language e.g: british */
    char *desc;                 /* Text description for preferences page */
};

struct config_template {
    struct config_template *next; /* Linked list                    */
    char *name;                 /* Short name of this theme       */
    char *description;          /* Description for prefs screen   */
};

struct config_theme {
    struct config_theme *next;  /* Linked list                    */
    char *name;                 /* Short name of this theme       */
    char *description;          /* Description for prefs screen   */
    char *fgcolor;              /* Default foreground colour      */
    char *fgcolor_link;         /* Default link foreground colour */
    char *bgcolor;              /* Default background colour      */
    char *bgcolor_banner;       /* Background for cmd banners     */
    char *bgcolor_row1;         /* Background for even rows       */
    char *bgcolor_row2;         /* Background for odd  rows       */
    char *bgcolor_status;       /* Background for status lines    */
    char *bgcolor_status_none;  /* Background for empty status    */
    char *fgcolor_quote1;       /* 1st level of quoting           */
    char *fgcolor_quote2;       /* 2nd level of quoting           */
    char *fgcolor_quote3;       /* 3rd level of quoting           */
    char *fgcolor_quote4;       /* 4th level of quoting           */
};

#define config_theme_check_bgcolor(theme, color) \
  (color && color[0] && (strcmp(theme->bgcolor, color) != 0))

struct config {
    struct pool *pool;          /* Private pool for config */
    /* Assorted paths */
    char *prefix;               /* (Optional) read-only prefix */
    char *var_prefix;           /* (Optional) read-write prefix */
    BOOL  raven_enable;         /* Enable raven login */
    char *raven_key_path;       /* Path to raven keys */
    char *login_insert1_path;   /* Chunks of HTML to insert into login screen */
    char *login_insert2_path;   /* Chunks of HTML to insert into login screen */
    char *login_template;       /* Template to use on login screen */
    char *motd_path;            /* Message of the day filename */
    char *welcome_path;         /* Welcome message */
    char *icon_dir;             /* Icon directory */
    char *static_dir;           /* Dir with Static entites: HTML,CSS */
    unsigned long static_expire_timeout;  /* Expires:for static files  (secs) */
    char *socket_dir;           /* Socket Directory */
    BOOL socket_split_dir;      /* Split sockets across directories */
    char *ssl_cipher_list;       /* List of ciphers */
    BOOL  ssl_server_preference; /* Server selects preferred cipher */
    char *ssl_session_dir;      /* SSL session directory */
    char *lock_dir;             /* Lock Directory */
    char *log_dir;              /* Log directory */
    char *tmp_dir;              /* Directory for temporary files */
    char *pid_dir;              /* Directory for PID files */
    char *bin_dir;              /* Location of binaries */
    char *init_socket_name;     /* Name of socket within socketdir */
    unsigned long file_perms;   /* For new read-write files */
    unsigned long directory_perms;      /* For new read-write directoies */
    BOOL check_directory_perms; /* T => check read-write */
    /* file and directory permissions */
    BOOL log_debug;             /* T => Enable debug logging    */
    BOOL fatal_dump_core;       /* fatal() should dump core?    */

    char *template_path;        /* Where to find raw template files */
    char *template_set;         /* (Default?) template set to use */
    BOOL  template_use_compiled; /* Use compiled templates */

    /* Frontend server configuration */
    char *prayer_user;          /* User that prayer will run as */
    uid_t prayer_uid;           /* Equivalent UID after lookup */
    char *prayer_group;         /* Group that prayer will run as */
    gid_t prayer_gid;           /* Equivalent GID after lookup */
    BOOL prayer_background;     /* Master prayer should fork and exit */

    char *hostname_canonical;   /* Overrides gethostbyname            */
    char *hostname;             /* Overrides gethostbyname            */
    char *hostname_service;     /* Login screen, overrides hostname   */
    BOOL  referer_block_invalid;/* Block logins from unknown referers */
    BOOL  referer_log_invalid;  /* Log invalid referrer headers       */
    BOOL fix_client_ipaddr;     /* Client must login from single addr */
    char *gzip_allow_nets;      /* Enable  gzip from this list        */
    char *gzip_deny_nets;       /* Disable gzip from this list        */
    char *log_name_nets;        /* Reverse lookup, log name for nets  */

    /* Emergency backstops */
    unsigned long limit_vm;     /* Limit on single prayer or prayer-session */

    unsigned long recips_max_msg;
    unsigned long recips_max_session;
    char *sending_allow_dir;            /* Override block_sending     */
    char *sending_block_dir;            /* Disable sending on account */

    /* HTTP configuration */
    BOOL http_cookie_use_port;  /* Use username:port in cookie       */
    unsigned long http_timeout_idle;    /* In seconds */
    unsigned long http_timeout_icons;   /* In seconds */
    unsigned long http_timeout_session; /* In seconds */
    unsigned long http_min_servers;     /* Minimum spare frontend servers */
    unsigned long http_max_servers;     /* Maximum spare frontend servers */
    unsigned long http_max_connections; /* Maximum connections to f'end server */
    unsigned long http_max_method_size; /* In bytes   */
    unsigned long http_max_hdr_size;    /* In bytes   */
    unsigned long http_max_body_size;   /* In bytes   */
    BOOL          http_dump;            /* Temporary */

    struct list *http_port_list;        /* List of HTTP and HTTPS ports      */
    char *ssl_cert_file;        /* SSL Certificate file              */
    char *ssl_privatekey_file;  /* SSL Privatekey file               */
    char *ssl_dh_file;          /* SSL DH file                       */
    unsigned long ssl_session_timeout;  /* Timeout for SSL sessions          */
    unsigned long ssl_rsakey_lifespan;  /* Master server regenerates RSA key */
    unsigned long ssl_rsakey_freshen;   /* Keys last this long after 1st use */
    unsigned long ssl_default_port;     /* Default HTTPS port, if any        */
    BOOL ssl_encouraged;        /* Warn before login if not SSL      */
    BOOL ssl_redirect;          /* Redirect non-SSL sessions to SSL  */
    BOOL ssl_required;          /* Disable insecure login            */
    char *egd_socket;           /* Path for EGD socket               */

    /* Direct access configuration */
    unsigned long direct_enable;        /* Enable direct access to session   */
    unsigned long direct_ssl_first;     /* Direst SSL   Ports start here     */
    unsigned long direct_ssl_count;     /* Direct SSL   Ports available      */
    unsigned long direct_plain_first;   /* Direst Plain Ports start here     */
    unsigned long direct_plain_count;   /* Direct plain Ports available      */

    /* Various timeouts */
    unsigned long session_idle_time;    /* Session moves into idle mode    */
    unsigned long session_timeout_compose;
    /* Backend session timeout (compose) */
    unsigned long session_timeout;      /* Backend session timeout (secs)  */
    unsigned long icon_expire_timeout;  /* Expires: hdr for icons  (secs)  */
    unsigned long session_key_len;      /* Length of session key           */
    unsigned long stream_checkpoint;    /* Checkpoint or ping    live streams */
    unsigned long stream_ping_interval; /* C'point/Ping interval live streams */
    unsigned long stream_misc_timeout;  /* Timeouts for misc streams         */
    unsigned long log_ping_interval;    /* Ping log files at this interval */
    unsigned long db_ping_interval;     /* Ping db files at this interval  */

    /* Global defaults affecting look and feel (use to prime user prefs?)    */
    char *login_banner;         /* Login banner                     */
    char *login_service_name;   /* Login service name               */
    unsigned long list_addr_maxlen;     /* cmd_list: Max size of address    */
    unsigned long list_subject_maxlen;  /* cmd_list: Max size of subject    */
    struct list *local_domain_list;     /* List of local domains w/lookups  */
    unsigned long local_domain_time;    /* Time CDB files last reopened     */
    struct list *ispell_language_list;  /* List of languages for ispell     */
    char *filter_domain_pattern;        /* Filter pattern for local domains */
    unsigned long change_max_folders;   /* Max folders on change dialogue   */
    unsigned long draft_att_single_max; /* Limit on single attachment       */
    unsigned long draft_att_total_max;  /* Limit on all attachments        */

    /* Location of Backend IMAP server */
    char *imapd_user_map;       /* CDB map: username -> server[:port] */
    char *imapd_server;         /* Default server[:port] */
    char *prefs_folder_name;    /* Name of preferences file on server  */
    char *accountd_user_map;    /* CDB map: username -> server[:port] */
    char *accountd_server;      /* Accountd server  */
    char *accountd_nis_server;  /* Accountd NIS server  */
    unsigned long accountd_timeout;        /* Accountd Timeout */
    unsigned long accountd_passwd_delay;   /* Delay for passwd updates */
    unsigned long accountd_fullname_delay; /* Delay for fullname updates */
    unsigned long vacation_default_days;   /* Default :days option for sieve */
    char *sieved_user_map;        /* CDB map: username -> server[:port] */
    char *sieved_server;          /* Sieved server */
    unsigned long sieve_maxsize;  /* Cyrus default is 32K */
    unsigned long sieved_timeout; /* Sieved Timeout */
    char *sendmail_path;        /* Path to sendmail */
    char *aspell_path;          /* Path to aspell (Takes priority)  */
    char *ispell_path;          /* Path to ispell   */
    char *return_path_domain;   /* Return path domain */
    BOOL fix_from_address;      /* Stop user changing from address */
    unsigned long  spam_purge_timeout; /* Default value used by IMAP server */
    char *spam_purge_name;      /* Name of sieve folder used for spam purge */
    char *spam_purge_prefix;    /* Prefix to use in spam purge file */
    BOOL  strip_mail;           /* Strip mail/ from favourites, maybe others */

    char *lookup_fullname;      /* Map from userid to GeCOS fullname */
    char *lookup_rpasswd;       /* Finger database, GeCOS reverse lookup */
    char *lookup_rusername;     /* Finger database, Jackdaw reverse lookup */
    char *lookup_username;      /* Finger database, forward lookup */

    char *ldap_server;          /* LDAP server to use */
    char *ldap_base_dn;         /* LDAP base DN */
    unsigned long ldap_timeout; /* in seconds */

    struct list *template_list; /* List of defined templates */

    /* Theme stuff */
    struct list *theme_list;    /* List of defined themes */
    char *theme_default_main;   /* Name of default main theme     */
    char *theme_default_help;   /* Name of default help theme     */
    struct config_theme *theme_main;    /* Default theme for main text    */
    struct config_theme *theme_help;    /* Default theme for help text    */

    /* User preference defaults : see prefs.h for details */
    BOOL allow_raven_login;
    BOOL confirm_expunge;
    BOOL expunge_on_exit;
    BOOL confirm_logout;
    BOOL confirm_rm;
    unsigned long msgs_per_page;
    unsigned long msgs_per_page_min;
    unsigned long msgs_per_page_max;
    unsigned long abook_per_page;
    unsigned long abook_per_page_min;
    unsigned long abook_per_page_max;
    BOOL suppress_dotfiles;
    char *maildir;
    BOOL  use_namespace;
    char *personal_hierarchy;
    char *hiersep;
    BOOL dualuse;
    char *postponed_folder;
    char *sent_mail_folder;
    char *default_domain;
    char *ispell_language;
    BOOL  spell_skip_quoted;    /* aspell only */
    unsigned long small_cols;
    unsigned long small_rows;
    unsigned long large_cols;
    unsigned long large_rows;
    char *sort_mode;
    BOOL sort_reverse;
    char *abook_sort_mode;
    BOOL abook_sort_reverse;
    unsigned long line_wrap_len;
    BOOL line_wrap_advanced;
    BOOL line_wrap_on_reply;
    BOOL line_wrap_on_spell;
    BOOL line_wrap_on_send;
    BOOL use_sent_mail;
    BOOL use_search_zoom;
    BOOL use_mark_persist;
    BOOL use_agg_unmark;
    BOOL use_icons;
    BOOL use_welcome;
    BOOL use_unread;
    BOOL use_tail_banner;
    BOOL use_cookie;
    BOOL use_substitution;
    BOOL use_http_1_1;
    BOOL use_pipelining;
    BOOL use_persist;
    BOOL use_short;
    BOOL use_gzip;
    BOOL html_inline;
    BOOL html_inline_auto;
    BOOL html_remote_images;
    BOOL preserve_mimetype;
};

#define CONFIG_PREFERRED_POOL_SIZE      (2048)
#define CONFIG_PREFERRED_TEMP_POOL_SIZE (8192)

struct config *config_create(void);
void config_free(struct config *config);
struct config *config_clone_parsed(struct config *src);

BOOL config_parse_file(struct config *config, char *filename);
BOOL config_parse_option(struct config *config, char *option);
BOOL config_expand(struct config *config);
BOOL config_check(struct config *config);
BOOL config_mkdirs(struct config *config);
BOOL config_local_domain_open(struct config *config);

void config_extract_ssl(struct config *config, struct ssl_config *ssl_config);
