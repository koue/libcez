/* $Cambridge: hermes/src/prayer/shared/config.c,v 1.12 2012/06/30 14:30:08 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Class to manage Prayer configuration */

/* ====================================================================== */

/* Code to compute canonical hostname, stolen from PINE */

/*----------------------------------------------------------------------
       Get the current host and domain names

    Args: hostname   -- buffer to return the hostname in
          hsize      -- size of buffer above
          domainname -- buffer to return domain name in
          dsize      -- size of buffer above

  Result: The system host and domain names are returned. If the full host
          name is akbar.cac.washington.edu then the domainname is
          cac.washington.edu.

On Internet connected hosts this look up uses /etc/hosts and DNS to
figure all this out. On other less well connected machines some other
file may be read. If there is no notion of a domain name the domain
name may be left blank. On a PC where there really isn't a host name
this should return blank strings. The .pinerc will take care of
configuring the domain names. That is, this should only return the
native system's idea of what the names are if the system has such
a concept.
 ----*/

#define MAX_ADDRESS (511)

static void
getdomainnames(char *hostname, int hsize, char *domainname, int dsize)
{
    char           *dn, hname[MAX_ADDRESS+1];
    struct hostent *he;
    char          **alias;
    char           *maybe = NULL;

    gethostname(hname, MAX_ADDRESS);
    he = gethostbyname(hname);
    hostname[0] = '\0';

    if(he == NULL){
	strncpy(hostname, hname, hsize-1);
	hostname[hsize-1] = '\0';
    }
    else{
	/*
	 * If no dot in hostname it may be the case that there
	 * is an alias which is really the fully-qualified
	 * hostname. This could happen if the administrator has
	 * (incorrectly) put the unqualified name first in the
	 * hosts file, for example. The problem with looking for
	 * an alias with a dot is that now we're guessing, since
	 * the aliases aren't supposed to be the official hostname.
	 * We'll compromise and only use an alias if the primary
	 * name has no dot and exactly one of the aliases has a
	 * dot.
	 */
	strncpy(hostname, he->h_name, hsize-1);
	hostname[hsize-1] = '\0';
	if (strchr(hostname, '.') == NULL) { /* no dot in hostname */
	    for(alias = he->h_aliases; *alias; alias++){
		if(strchr(*alias, '.') != NULL){	/* found one */
		    if(maybe){		/* oops, this is the second one */
			maybe = NULL;
			break;
		    }
		    else
		      maybe = *alias;
		}
	    }

	    if(maybe){
		strncpy(hostname, maybe, hsize-1);
		hostname[hsize-1] = '\0';
	    }
	}
    }

    hostname[hsize-1] = '\0';

    if((dn = strchr(hostname, '.')) != NULL)
      strncpy(domainname, dn+1, dsize-1);
    else
      strncpy(domainname, hostname, dsize-1);

    domainname[dsize-1] = '\0';
}

/* ====================================================================== */

struct config_theme *config_theme_create(struct pool *pool)
{
    struct config_theme *theme =
        pool_alloc(pool, sizeof(struct config_theme));

    memset(theme, 0, sizeof(struct config_theme));
    theme->name = NIL;          /* Theme Name                 */
    theme->description = NIL;   /* Theme description          */
    theme->fgcolor = NIL;       /* Default foreground colour  */
    theme->fgcolor = NIL;       /* Default link colour        */
    theme->bgcolor = NIL;       /* Default background colour  */
    theme->bgcolor_banner = NIL;        /* Background for cmd banners */
    theme->bgcolor_row1 = NIL;  /* Background for even rows   */
    theme->bgcolor_row2 = NIL;  /* Background for odd  rows   */
    theme->bgcolor_status = NIL;        /* Background for status lines   */
    theme->bgcolor_status_none = NIL;   /* Background empty status lines */
    theme->fgcolor_quote1 = NIL;        /* 1st level of quoting          */
    theme->fgcolor_quote2 = NIL;        /* 2nd level of quoting          */
    theme->fgcolor_quote3 = NIL;        /* 3rd level of quoting          */
    theme->fgcolor_quote4 = NIL;        /* 4th level of quoting          */

    return (theme);
}

/* ====================================================================== */

/* config_create() *********************************************************
 *
 * Create a new config structure using own pool
 **************************************************************************/

struct config *config_create(void)
{
    struct pool *pool = pool_create(CONFIG_PREFERRED_POOL_SIZE);
    struct config *config = pool_alloc(pool, sizeof(struct config));

    memset(config, 0, sizeof(struct config));
    config->pool = pool;
    config->prefix = NIL;
    config->var_prefix = NIL;
    config->raven_enable = NIL;
    config->raven_key_path = "";
    config->login_insert1_path = NIL;
    config->login_insert2_path = NIL;
    config->login_template = "login";
    config->motd_path = NIL;
    config->welcome_path = NIL;
    config->icon_dir = NIL;
    config->static_dir = NIL;
    config->static_expire_timeout = (7 * 24 * 60 * 60);   /* 7 days  */
    config->socket_dir = NIL;
    config->socket_split_dir = NIL;
    config->ssl_cipher_list = NIL;
    config->ssl_server_preference = NIL;
    config->ssl_session_dir = NIL;
    config->lock_dir = NIL;
    config->log_dir = NIL;
    config->tmp_dir = NIL;
    config->pid_dir = NIL;
    config->bin_dir = NIL;
    config->init_socket_name = NIL;
    config->file_perms = 0640;
    config->directory_perms = 0750;
    config->check_directory_perms = NIL;

    config->log_debug = NIL;
    config->fatal_dump_core = NIL;

    config->template_path = "../templates";
    config->template_set  = "old";
    config->template_use_compiled = T;
    config->template_list = NIL;

    config->prayer_user = NIL;
    config->prayer_uid = 0;
    config->prayer_group = NIL;
    config->prayer_gid = 0;
    config->prayer_background = T;
    config->hostname = NIL;
    config->hostname_service = NIL;
    config->hostname_canonical = NIL;
    config->referer_log_invalid   = T;
    config->referer_block_invalid = NIL;
    config->fix_client_ipaddr = NIL;
    config->gzip_allow_nets = NIL;
    config->gzip_deny_nets = NIL;
    config->log_name_nets = NIL;

    config->limit_vm = 0;
    config->recips_max_msg = 0;
    config->recips_max_session = 0;
    config->sending_allow_dir = NIL;
    config->sending_block_dir = NIL;

    config->http_cookie_use_port = NIL;
    config->http_max_method_size = 0;
    config->http_max_hdr_size = 0;
    config->http_max_body_size = 0;
    config->http_port_list = NIL;
    config->http_timeout_idle = 30;
    config->http_timeout_icons = 10;
    config->http_timeout_session = 60;
    config->http_min_servers = 4;
    config->http_max_servers = 64;
    config->http_max_connections = 0;
    config->http_dump = NIL;

    config->ssl_cert_file = NIL;
    config->ssl_privatekey_file = NIL;
    config->ssl_dh_file = NIL;
    config->ssl_rsakey_lifespan = 15 * 60;
    config->ssl_rsakey_freshen = 15 * 60;
    config->ssl_session_timeout = 0;    /* Disable session cache */
    config->ssl_default_port = 0;
    config->ssl_encouraged = NIL;
    config->ssl_redirect = NIL;
    config->ssl_required = NIL;
    config->egd_socket = NIL;

    config->direct_enable = NIL;
    config->direct_ssl_first = 0;
    config->direct_ssl_count = 0;
    config->direct_plain_first = 0;
    config->direct_plain_count = 0;

    config->session_idle_time = 0;      /* Disable idle mode */
    config->session_timeout = (4 * 60 * 60);    /* 4 hours */
    config->session_timeout_compose = 0;        /* Same as main timeout */
    config->icon_expire_timeout = (7 * 24 * 60 * 60);   /* 7 days  */
    config->session_key_len = 18;
    config->stream_checkpoint = T;
    config->stream_misc_timeout = 0;
    config->stream_ping_interval = (5 * 60);    /* 5 mins  */
    config->log_ping_interval = (5 * 60);       /* 5 mins  */
    config->db_ping_interval = (30 * 60);       /* 30 mins */

    config->login_banner = NIL;
    config->login_service_name = NIL;
    config->list_addr_maxlen = 30;
    config->list_subject_maxlen = 30;
    config->local_domain_list = NIL;
    config->local_domain_time = 0;
    config->filter_domain_pattern = NIL;
    config->change_max_folders = 20;
    config->draft_att_single_max = 0;
    config->draft_att_total_max = 0;

    config->imapd_user_map = NIL;
    config->imapd_server = NIL;
    config->prefs_folder_name = NIL;
    config->accountd_user_map = NIL;
    config->accountd_server = NIL;
    config->accountd_nis_server = NIL;
    config->accountd_passwd_delay = 0;
    config->accountd_fullname_delay = 0;
    config->vacation_default_days = 31;
    config->accountd_timeout = NIL;
    config->sieve_maxsize = 0;
    config->sieved_user_map = NIL;
    config->sieved_server = NIL;
    config->sieved_timeout = (9*60);   /* Seconds */
    config->sendmail_path = "/usr/lib/sendmail";
    config->aspell_path = NIL;
    config->ispell_path = NIL;
    config->return_path_domain = NIL;
    config->fix_from_address = NIL;
    config->spam_purge_timeout = 60;
    config->spam_purge_name   = "spam_purge";
    config->spam_purge_prefix = "# Spam Purge Timeout:";
    config->strip_mail = NIL;

    config->lookup_fullname  = NIL;
    config->lookup_rpasswd   = NIL;
    config->lookup_rusername = NIL;
    config->lookup_username  = NIL;

    config->ldap_server      = NIL;
    config->ldap_base_dn     = NIL;
    config->ldap_timeout     = 0;

    config->theme_list = NIL;
    config->theme_default_main = NIL;
    config->theme_default_help = NIL;
    config->theme_main = NIL;
    config->theme_help = NIL;

    config->allow_raven_login = T;
    config->confirm_expunge = NIL;
    config->confirm_logout = T;
    config->confirm_rm = T;
    config->expunge_on_exit = NIL;
    config->msgs_per_page = 12;
    config->msgs_per_page_max = 50;
    config->msgs_per_page_min = 4;
    config->abook_per_page = 12;
    config->abook_per_page_max = 50;
    config->abook_per_page_min = 4;
    config->suppress_dotfiles = T;
    config->use_namespace = T;
    config->maildir = "";
    config->personal_hierarchy = "";
    config->hiersep = "/";
    config->dualuse = NIL;
    config->postponed_folder = "postponed-msgs";
    config->sent_mail_folder = "sent-mail";
    config->default_domain = NIL;
    config->ispell_language = "british";
    config->spell_skip_quoted = T;
    config->small_cols = 80;
    config->small_rows = 18;
    config->large_cols = 80;
    config->large_rows = 32;
    config->sort_mode = "ARRIVAL";
    config->sort_reverse = NIL;
    config->abook_sort_mode    = "ORDERED";
    config->abook_sort_reverse = NIL;

    config->line_wrap_len = 76;
    config->line_wrap_advanced = NIL;
    config->line_wrap_on_reply = T;
    config->line_wrap_on_spell = T;
    config->line_wrap_on_send = T;

    config->use_sent_mail = T;
    config->use_search_zoom = T;
    config->use_agg_unmark = T;
    config->use_icons = T;
    config->use_welcome = T;
    config->use_unread = T;
    config->use_tail_banner = T;
    config->use_mark_persist = NIL;
    config->use_cookie = T;
    config->use_substitution = T;
    config->use_http_1_1 = T;
    config->use_pipelining = T;
    config->use_persist = T;
    config->use_short = T;
    config->use_gzip = T;
    config->html_inline = T;
    config->html_inline_auto = T;
    config->html_remote_images = NIL;
    config->preserve_mimetype = T;

    return (config);
}

/* config_free() *********************************************************
 *
 * Free config structure
 ************************************************************************/

void config_free(struct config *config)
{
    pool_free(config->pool);
}

/* ====================================================================== */

/* config_paniclog() *******************************************************
 *
 * Special interation of paniclog() to print errrors consistently when
 * frontend or session starting up with introducing dependancies on code
 * which is specific to the frontend or session processes.
 **************************************************************************/

static void config_paniclog(struct config *config, char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    va_start(ap, fmt);
    len = log_entry_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_panic_ap(config, NIL, len, fmt, ap);
    va_end(ap);
}

/* ====================================================================== */

/* conf used only to calculate offsets into live config structure */
static struct config conf;
#define OFFSET(x) ((char*)(&conf.x) - (char *)&conf)

/* Config options. Searched by binary chop => must be sorted correctly
 * However config_init() checks that order is correct, bails out otherwise */
static struct {
    char *name;
    enum { config_bool, config_number, config_time, config_string,
        config_path, config_list, config_perm, config_unknown
    } type;
    int offset;
} config_options[] = {
    {
    "abook_per_page", config_number, OFFSET(abook_per_page)}
    , {
    "abook_per_page_max", config_number, OFFSET(abook_per_page_max)}
    , {
    "abook_per_page_min", config_number, OFFSET(abook_per_page_min)}
    , {
    "abook_sort_mode", config_string, OFFSET(abook_sort_mode)}
    , {
    "abook_sort_reverse", config_bool, OFFSET(abook_sort_reverse)}
    , {
    "accountd_fullname_delay", config_time,
            OFFSET(accountd_fullname_delay)}
    , {
    "accountd_nis_server", config_string, OFFSET(accountd_nis_server)}
    , {
    "accountd_passwd_delay", config_time, OFFSET(accountd_passwd_delay)}
    , {
    "accountd_server", config_string, OFFSET(accountd_server)}
    , {
    "accountd_timeout", config_time, OFFSET(accountd_timeout)}
    , {
    "accountd_user_map", config_path, OFFSET(accountd_user_map)}
    , {
    "allow_raven_login", config_bool, OFFSET(allow_raven_login)}
    , {
    "aspell_path", config_path, OFFSET(aspell_path)}
    , {
    "bin_dir", config_path, OFFSET(bin_dir)}
    , {
    "change_max_folders", config_number, OFFSET(change_max_folders)}
    , {
    "check_directory_perms", config_bool, OFFSET(check_directory_perms)}
    , {
    "confirm_expunge", config_bool, OFFSET(confirm_expunge)}
    , {
    "confirm_logout", config_bool, OFFSET(confirm_logout)}
    , {
    "confirm_rm", config_bool, OFFSET(confirm_rm)}
    , {
    "db_ping_interval", config_time, OFFSET(db_ping_interval)}
    , {
    "default_domain", config_string, OFFSET(default_domain)}
    , {
    "direct_enable", config_bool, OFFSET(direct_enable)}
    , {
    "direct_plain_count", config_number, OFFSET(direct_plain_count)}
    , {
    "direct_plain_first", config_number, OFFSET(direct_plain_first)}
    , {
    "direct_ssl_count", config_number, OFFSET(direct_ssl_count)}
    , {
    "direct_ssl_first", config_number, OFFSET(direct_ssl_first)}
    , {
    "directory_perms", config_perm, OFFSET(directory_perms)}
    , {
    "draft_att_single_max", config_number, OFFSET(draft_att_single_max)}
    , {
    "draft_att_total_max", config_number, OFFSET(draft_att_total_max)}
    , {
    "dualuse", config_bool, OFFSET(dualuse)}
    , {
    "egd_socket", config_path, OFFSET(egd_socket)}
    , {
    "expunge_on_exit", config_bool, OFFSET(expunge_on_exit)}
    , {
    "fatal_dump_core", config_bool, OFFSET(fatal_dump_core)}
    , {
    "file_perms", config_perm, OFFSET(file_perms)}
    , {
    "filter_domain_pattern", config_string,
            OFFSET(filter_domain_pattern)}
    , {
    "fix_client_ipaddr", config_bool, OFFSET(fix_client_ipaddr)}
    , {
    "fix_from_address", config_bool, OFFSET(fix_from_address)}
    , {
    "gzip_allow_nets", config_string, OFFSET(gzip_allow_nets)}
    , {
    "gzip_deny_nets", config_string, OFFSET(gzip_deny_nets)}
    , {
    "hiersep", config_string, OFFSET(hiersep)}
    , {
    "hostname", config_string, OFFSET(hostname)}
    , {
    "hostname_canonical", config_string, OFFSET(hostname_canonical)}
    , {
    "hostname_service", config_string, OFFSET(hostname_service)}
    , {
    "html_inline", config_bool, OFFSET(html_inline)}
    , {
    "html_inline_auto", config_bool, OFFSET(html_inline_auto)}
    , {
    "html_remote_images", config_bool, OFFSET(html_remote_images)}
    , {
    "http_cookie_use_port", config_bool, OFFSET(http_cookie_use_port)}
    , {
    "http_dump", config_bool, OFFSET(http_dump)}
    , {
    "http_max_body_size", config_number, OFFSET(http_max_body_size)}
    , {
    "http_max_connections", config_number, OFFSET(http_max_connections)}
    , {
    "http_max_hdr_size", config_number, OFFSET(http_max_hdr_size)}
    , {
    "http_max_method_size", config_number, OFFSET(http_max_method_size)}
    , {
    "http_max_servers", config_number, OFFSET(http_max_servers)}
    , {
    "http_min_servers", config_number, OFFSET(http_min_servers)}
    , {
    "http_timeout_icons", config_time, OFFSET(http_timeout_icons)}
    , {
    "http_timeout_idle", config_time, OFFSET(http_timeout_idle)}
    , {
    "http_timeout_session", config_time, OFFSET(http_timeout_session)}
    , {
    "icon_dir", config_path, OFFSET(icon_dir)}
    , {
    "icon_expire_timeout", config_time, OFFSET(icon_expire_timeout)}
    , {
    "imapd_server", config_string, OFFSET(imapd_server)}
    , {
    "imapd_user_map", config_path, OFFSET(imapd_user_map)}
    , {
    "init_socket_name", config_string, OFFSET(init_socket_name)}
    , {
    "ispell_language", config_string, OFFSET(ispell_language)}
    , {
    "ispell_path", config_path, OFFSET(ispell_path)}
    , {
    "large_cols", config_number, OFFSET(large_cols)}
    , {
    "large_rows", config_number, OFFSET(large_rows)}
    , {
    "ldap_base_dn", config_string, OFFSET(ldap_base_dn)}
    , {
    "ldap_server", config_string, OFFSET(ldap_server)}
    , {
    "ldap_timeout", config_time , OFFSET(ldap_timeout)}
    , {
    "limit_vm", config_number, OFFSET(limit_vm)}
    , {
    "line_wrap_advanced", config_bool, OFFSET(line_wrap_advanced)}
    , {
    "line_wrap_len", config_number, OFFSET(line_wrap_len)}
    , {
    "line_wrap_on_reply", config_bool, OFFSET(line_wrap_on_reply)}
    , {
    "line_wrap_on_send", config_bool, OFFSET(line_wrap_on_send)}
    , {
    "line_wrap_on_spell", config_bool, OFFSET(line_wrap_on_spell)}
    , {
    "list_addr_maxlen", config_number, OFFSET(list_addr_maxlen)}
    , {
    "list_subject_maxlen", config_number, OFFSET(list_subject_maxlen)}
    , {
    "local_domain_list", config_list, OFFSET(local_domain_list)}
    , {
    "lock_dir", config_path, OFFSET(lock_dir)}
    , {
    "log_debug", config_bool, OFFSET(log_debug)}
    , {
    "log_dir", config_path, OFFSET(log_dir)}
    , {
    "log_name_nets", config_string, OFFSET(log_name_nets)}
    , {
    "log_ping_interval", config_time, OFFSET(log_ping_interval)}
    , {
    "login_banner", config_string, OFFSET(login_banner)}
    , {
    "login_insert1_path", config_path, OFFSET(login_insert1_path)}
    , {
    "login_insert2_path", config_path, OFFSET(login_insert2_path)}
    , {
    "login_service_name", config_string, OFFSET(login_service_name)}
    , {
    "login_template", config_string, OFFSET(login_template)}
    , {
    "lookup_fullname", config_path, OFFSET(lookup_fullname)}
    , {
    "lookup_rpasswd", config_path, OFFSET(lookup_rpasswd)}
    , {
    "lookup_rusername", config_path, OFFSET(lookup_rusername)}
    , {
    "lookup_username", config_path, OFFSET(lookup_username)}
    , {
    "maildir", config_string, OFFSET(maildir)}
    , {
    "motd_path", config_path, OFFSET(motd_path)}
    , {
    "msgs_per_page", config_number, OFFSET(msgs_per_page)}
    , {
    "msgs_per_page_max", config_number, OFFSET(msgs_per_page_max)}
    , {
    "msgs_per_page_min", config_number, OFFSET(msgs_per_page_min)}
    , {
    "personal_hierarchy", config_string, OFFSET(personal_hierarchy)}
    , {
    "pid_dir", config_path, OFFSET(pid_dir)}
    , {
    "postponed_folder", config_string, OFFSET(postponed_folder)}
    , {
    "prayer_background", config_bool, OFFSET(prayer_background)}
    , {
    "prayer_gid", config_number, OFFSET(prayer_gid)}
    , {
    "prayer_group", config_string, OFFSET(prayer_group)}
    , {
    "prayer_uid", config_number, OFFSET(prayer_uid)}
    , {
    "prayer_user", config_string, OFFSET(prayer_user)}
    , {
    "prefix", config_string, OFFSET(prefix)}
    , {
    "prefs_folder_name", config_string, OFFSET(prefs_folder_name)}
    , {
    "preserve_mimetype", config_bool, OFFSET(preserve_mimetype)}
    , {
    "raven_enable", config_bool, OFFSET(raven_enable)}
    , {
    "raven_key_path", config_path, OFFSET(raven_key_path)}
    , {
    "recips_max_msg", config_number, OFFSET(recips_max_msg)}
    , {
    "recips_max_session", config_number, OFFSET(recips_max_session)}
    , {
    "referer_block_invalid", config_bool, OFFSET(referer_block_invalid)}
    , {
    "referer_log_invalid", config_bool, OFFSET(referer_log_invalid)}
    , {
    "return_path_domain", config_string, OFFSET(return_path_domain)}
    , {
    "sending_allow_dir", config_path, OFFSET(sending_allow_dir)}
    , {
    "sending_block_dir", config_path, OFFSET(sending_block_dir)}
    , {
    "sendmail_path", config_path, OFFSET(sendmail_path)}
    , {
    "sent_mail_folder", config_string, OFFSET(sent_mail_folder)}
    , {
    "session_idle_time", config_time, OFFSET(session_idle_time)}
    , {
    "session_key_len", config_number, OFFSET(session_key_len)}
    , {
    "session_timeout", config_time, OFFSET(session_timeout)}
    , {
    "session_timeout_compose", config_time,
            OFFSET(session_timeout_compose)}
    , {
    "sieve_maxsize", config_number, OFFSET(sieve_maxsize)}
    , {
    "sieved_server", config_string, OFFSET(sieved_server)}
    , {
    "sieved_timeout", config_time, OFFSET(sieved_timeout)}
    , {
    "sieved_user_map", config_path, OFFSET(sieved_user_map)}
    , {
    "small_cols", config_number, OFFSET(small_cols)}
    , {
    "small_rows", config_number, OFFSET(small_rows)}
    , {
    "socket_dir", config_path, OFFSET(socket_dir)}
    , {
    "socket_split_dir", config_bool, OFFSET(socket_split_dir)}
    , {
    "sort_mode", config_string, OFFSET(sort_mode)}
    , {
    "sort_reverse", config_bool, OFFSET(sort_reverse)}
    , {
    "spam_purge_name", config_string, OFFSET(spam_purge_name)}
    , {
    "spam_purge_prefix", config_string, OFFSET(spam_purge_prefix)}
    , {
    "spam_purge_timeout", config_number, OFFSET(spam_purge_timeout)}
    , {
    "spell_skip_quoted", config_bool, OFFSET(spell_skip_quoted)}
    , {
    "ssl_cert_file", config_path, OFFSET(ssl_cert_file)}
    , {
    "ssl_cipher_list", config_string, OFFSET(ssl_cipher_list)}
    , {
    "ssl_default_port", config_number, OFFSET(ssl_default_port)}
    , {
    "ssl_dh_file", config_path, OFFSET(ssl_dh_file)}
    , {
    "ssl_encouraged", config_bool, OFFSET(ssl_encouraged)}
    , {
    "ssl_privatekey_file", config_path, OFFSET(ssl_privatekey_file)}
    , {
    "ssl_redirect", config_bool, OFFSET(ssl_redirect)}
    , {
    "ssl_required", config_bool, OFFSET(ssl_required)}
    , {
    "ssl_rsakey_freshen", config_time, OFFSET(ssl_rsakey_freshen)}
    , {
    "ssl_rsakey_lifespan", config_time, OFFSET(ssl_rsakey_lifespan)}
    , {
    "ssl_server_preference", config_bool, OFFSET(ssl_server_preference)}
    , {
    "ssl_session_dir", config_path, OFFSET(ssl_session_dir)}
    , {
    "ssl_session_timeout", config_time, OFFSET(ssl_session_timeout)}
    , {
    "static_dir", config_path, OFFSET(static_dir)}
    , {
    "static_expire_timeout", config_time, OFFSET(static_expire_timeout)}
    , {
    "stream_checkpoint", config_bool, OFFSET(stream_checkpoint)}
    , {
    "stream_misc_timeout", config_time, OFFSET(stream_misc_timeout)}
    , {
    "stream_ping_interval", config_time, OFFSET(stream_ping_interval)}
    , {
    "strip_mail", config_bool, OFFSET(strip_mail)}
    , {
    "suppress_dotfiles", config_bool, OFFSET(suppress_dotfiles)}
    , {
    "template_path", config_string, OFFSET(template_path)}
    , {
    "template_set", config_string, OFFSET(template_set)}
    , {
    "template_use_compiled", config_bool, OFFSET(template_use_compiled)}
    , {
    "theme_default_help", config_string, OFFSET(theme_default_help)}
    , {
    "theme_default_main", config_string, OFFSET(theme_default_main)}
    , {
    "tmp_dir", config_path, OFFSET(tmp_dir)}
    , {
    "use_agg_unmark", config_bool, OFFSET(use_agg_unmark)}
    , {
    "use_cookie", config_bool, OFFSET(use_cookie)}
    , {
    "use_gzip", config_bool, OFFSET(use_gzip)}
    , {
    "use_http_1_1", config_bool, OFFSET(use_http_1_1)}
    , {
    "use_icons", config_bool, OFFSET(use_icons)}
    , {
    "use_mark_persist", config_bool, OFFSET(use_mark_persist)}
    , {
    "use_namespace", config_bool, OFFSET(use_namespace)}
    , {
    "use_persist", config_bool, OFFSET(use_persist)}
    , {
    "use_pipelining", config_bool, OFFSET(use_pipelining)}
    , {
    "use_search_zoom", config_bool, OFFSET(use_search_zoom)}
    , {
    "use_sent_mail", config_bool, OFFSET(use_sent_mail)}
    , {
    "use_short", config_bool, OFFSET(use_short)}
    , {
    "use_substitution", config_bool, OFFSET(use_substitution)}
    , {
    "use_tail_banner", config_bool, OFFSET(use_tail_banner)}
    , {
    "use_unread", config_bool, OFFSET(use_unread)}
    , {
    "use_welcome", config_bool, OFFSET(use_welcome)}
    , {
    "vacation_default_days", config_number, OFFSET(vacation_default_days)}
    , {
    "var_prefix", config_string, OFFSET(var_prefix)}
    , {
    "welcome_path", config_path, OFFSET(welcome_path)}
    , {
    NIL, config_unknown, NIL}
};

static unsigned long options_size = 0L;

/* ====================================================================== */

/* config_init() *********************************************************
 *
 * Initialise config engine. Counts number of options in the "options"
 * array, and makes sure that they are properly sorted for binary chop
 * searches.
 ************************************************************************/

static BOOL config_init(void)
{
    unsigned long offset;

    offset = 0;
    while (config_options[offset].name) {
        if (config_options[offset + 1].name) {
            if (strcmp(config_options[offset].name,
                       config_options[offset + 1].name) > 0) {
                config_paniclog(NIL,
                                "config.c: options array sorted incorrectly at %s",
                                config_options[offset].name);
                return (NIL);
            }
        }
        offset++;
    }
    options_size = offset;
    return (T);
}

/* ====================================================================== */

/* config_find_key() *****************************************************
 *
 * Find offset to "options" structure which corresponds to key
 *    key: Key to lookup
 *
 * Returns: Array offset, (-1) on error.
 ************************************************************************/

static int config_find_option(char *key)
{
    int first = 0;
    int last = options_size;
    int middle = 0;
    int rc;

    /* Binary chop */
    while (first < last) {
        middle = (first + last) / 2;
        rc = strcmp(config_options[middle].name, key);

        if (rc == 0)
            return (middle);
        else if (rc < 0)
            first = middle + 1;
        else
            last = middle;
    }

    /* Check whether last interaction of loop found match */
    if (!strcmp(config_options[middle].name, key))
        return (middle);

    return (-1);                /* Not found */
}

/* ====================================================================== */

/* Various static support functions for config_clone */

static char *maybe_pool_strdup(struct pool *pool, char *s)
{
    return ((s) ? pool_strdup(pool, s) : NIL);
}

static void config_clone_bool(BOOL * dst, BOOL * src)
{
    *dst = *src;
}

static void config_clone_number(unsigned long *dst, unsigned long *src)
{
    *dst = *src;
}

static void config_clone_string(struct pool *pool, char **dst, char **src)
{
    *dst = maybe_pool_strdup(pool, *src);
}

static void
config_clone_http_port(struct config *config, struct config_http_port *src)
{
    struct config_http_port *dst = pool_alloc(config->pool,
                                              sizeof(struct
                                                     config_http_port));

    dst->next = NIL;

    if (src->interface)
        dst->interface = pool_strdup(config->pool, src->interface);
    else
        dst->interface = NIL;

    dst->port = src->port;
    dst->use_ssl = src->use_ssl;

    list_push(config->http_port_list, (struct list_item *) dst, src->name);
}

static void
config_clone_local_domain(struct config *config,
                          struct config_local_domain *src)
{
    struct config_local_domain *dst
        = pool_alloc(config->pool, sizeof(struct config_local_domain));

    dst->next = NIL;
    dst->file = maybe_pool_strdup(config->pool, src->file);
    dst->cdb_map = src->cdb_map;

    list_push(config->local_domain_list, (struct list_item *) dst,
              src->name);
}

static void
config_clone_language(struct config *config, struct config_language *src)
{
    struct config_language *dst
        = pool_alloc(config->pool, sizeof(struct config_language));

    dst->next = NIL;
    dst->desc = maybe_pool_strdup(config->pool, src->desc);

    list_push(config->ispell_language_list, (struct list_item *) dst,
              src->name);
}

static void
config_clone_template(struct config *config, struct config_template *src)
{
    struct pool *pool = config->pool;
    struct config_template *dst =
        pool_alloc(pool, sizeof(struct config_template));

    dst->description = maybe_pool_strdup(pool, src->description);

    list_push(config->template_list, (struct list_item *) dst, src->name);
}

static void
config_clone_theme(struct config *config, struct config_theme *src)
{
    struct pool *pool = config->pool;
    struct config_theme *dst =
        pool_alloc(pool, sizeof(struct config_theme));

    dst->description = maybe_pool_strdup(pool, src->description);
    dst->fgcolor = maybe_pool_strdup(pool, src->fgcolor);
    dst->fgcolor_link = maybe_pool_strdup(pool, src->fgcolor_link);
    dst->bgcolor = maybe_pool_strdup(pool, src->bgcolor);
    dst->bgcolor_banner = maybe_pool_strdup(pool, src->bgcolor_banner);
    dst->bgcolor_row1 = maybe_pool_strdup(pool, src->bgcolor_row2);
    dst->bgcolor_row2 = maybe_pool_strdup(pool, src->bgcolor_row1);
    dst->bgcolor_status = maybe_pool_strdup(pool, src->bgcolor_status);
    dst->bgcolor_status_none =
        maybe_pool_strdup(pool, src->bgcolor_status_none);
    dst->fgcolor_quote1 = maybe_pool_strdup(pool, src->fgcolor_quote1);
    dst->fgcolor_quote2 = maybe_pool_strdup(pool, src->fgcolor_quote2);
    dst->fgcolor_quote3 = maybe_pool_strdup(pool, src->fgcolor_quote3);
    dst->fgcolor_quote4 = maybe_pool_strdup(pool, src->fgcolor_quote4);

    list_push(config->theme_list, (struct list_item *) dst, src->name);
}

/* ====================================================================== */

/* config_clone_parsed() ***************************************************
 *
 * Take an existing config and clone it into a fresh pool. Used after
 * all setup, parsing and expansion has occured to give us a clean
 * config structure without any baggage.
 **************************************************************************/

struct config *config_clone_parsed(struct config *src)
{
    struct pool *pool = pool_create(CONFIG_PREFERRED_POOL_SIZE);
    struct config *config = pool_alloc(pool, sizeof(struct config));
    unsigned long offset;
    struct list_item *li;

    memset(config, 0, sizeof(struct config));
    config->pool = pool;

    offset = 0;
    while (config_options[offset].name) {
        void *s = ((char *) src) + config_options[offset].offset;
        void *d = ((char *) config) + config_options[offset].offset;

        switch (config_options[offset].type) {
        case config_bool:
            config_clone_bool((BOOL *) d, (BOOL *) s);
            break;
        case config_number:
        case config_time:
        case config_perm:
            config_clone_number((unsigned long *) d, (unsigned long *) s);
            break;
        case config_string:
        case config_path:
            config_clone_string(pool, (char **) d, (char **) s);
            break;
        default:
            /* Ignore config_list and config_unknown entries! */
            break;
        }
        offset++;
    }

    config->http_port_list = NIL;
    config->local_domain_list = NIL;
    config->ispell_language_list = NIL;
    config->template_list = NIL;
    config->theme_list = NIL;

    /* Clone HTTP Port List */
    if (src->http_port_list) {
        config->http_port_list = list_create(config->pool, T);
        for (li = src->http_port_list->head; li; li = li->next)
            config_clone_http_port(config, (struct config_http_port *) li);
    }

    /* Clone domain list */
    if (src->local_domain_list) {
        config->local_domain_list = list_create(config->pool, T);
        for (li = src->local_domain_list->head; li; li = li->next)
            config_clone_local_domain(config,
                                      (struct config_local_domain *) li);
    } else
        config->local_domain_list = NIL;

    /* Clone ispell language list */
    if (src->ispell_language_list) {
        config->ispell_language_list = list_create(config->pool, T);
        for (li = src->ispell_language_list->head; li; li = li->next)
            config_clone_language(config, (struct config_language *) li);
    } else
        config->ispell_language_list = NIL;

    /* Clone templates */
    if (src->template_list) {
        config->template_list = list_create(config->pool, T);
        for (li = src->template_list->head; li; li = li->next)
            config_clone_template(config, (struct config_template *) li);
    }

    /* Clone themes */
    config->theme_main = NIL;
    config->theme_help = NIL;

    if (src->theme_list) {
        config->theme_list = list_create(config->pool, T);
        for (li = src->theme_list->head; li; li = li->next)
            config_clone_theme(config, (struct config_theme *) li);

        /* Should be valid if we've got this far... */
        if (config->theme_default_main)
            config->theme_main = (struct config_theme *)
                list_lookup_byname(config->theme_list,
                                   config->theme_default_main);

        /* Should be valid if we've got this far... */
        if (config->theme_default_help)
            config->theme_help = (struct config_theme *)
                list_lookup_byname(config->theme_list,
                                   config->theme_default_help);
    } else
        config->theme_list = NIL;

    /* Make sure that theme_main points somewhere */
    if (config->theme_main == NIL)
        config->theme_main = config_theme_create(config->pool);

    /* Make sure that theme_help points somewhere */
    if (config->theme_help == NIL)
        config->theme_help = config_theme_create(config->pool);

    return (config);
}

/* ====================================================================== */

/* Various static utility routines for parsing elements of config file */

static BOOL config_parse_rest(char **textp)
{
    char *text = *textp;
    char *s;

    if (!(text && text[0]))
        return (NIL);

    text = string_trim_whitespace(text);

    if (*text == '"') {
        text++;
        for (s = text; *s; s++) {
            if (*s == '"') {
                *s++ = '\0';

                while (string_isspace(*s))
                    s++;

                if (*s && (*s != '#'))
                    return (NIL);

                *textp = text;
                return (T);
            }
        }
        return (NIL);
    }

    for (s = text; *s; s++) {
        if (*s == '#') {
            *s = '\0';
            break;
        }
    }

    *textp = text;
    return (T);
}

static BOOL config_parse_bool(BOOL * result, char *s)
{
    if (!config_parse_rest(&s))
        return (NIL);

    if (!strcasecmp(s, "TRUE") || !strcasecmp(s, "T") || !strcmp(s, "1")) {
        *result = T;
        return (T);
    }

    if (!strcasecmp(s, "FALSE") || !strcasecmp(s, "NIL")
        || !strcmp(s, "0")) {
        *result = NIL;
        return (T);
    }

    return (NIL);
}

static BOOL config_parse_number(unsigned long *result, char *text)
{
    char *s;
    unsigned long multiple = 1;

    if (!config_parse_rest(&text))
        return (NIL);

    for (s = text; s[1]; s++)
        if (!Uisdigit(*s))
            return (NIL);

    if (Uisdigit(*s))
        multiple = 1;
    else {
        switch (Utoupper(*s)) {
        case 'K':
            multiple = 1024;
            break;
        case 'M':
            multiple = (1024 * 1024);
            break;
        default:
            return (NIL);
        }
        *s++ = '\0';

        if (*s)
            return (NIL);
    }

    *result = (multiple * atoi(text));
    return (T);
}

static BOOL config_parse_perm(unsigned long *result, char *s)
{
    if (!config_parse_rest(&s))
        return (NIL);

    if (s[0] != '0')
        return (NIL);

    if ((s[1] < '0') || (s[1] > '7'))
        return (NIL);

    if ((s[2] < '0') || (s[2] > '7'))
        return (NIL);

    if ((s[3] < '0') || (s[3] > '7'))
        return (NIL);

    if (s[4])
        return (NIL);

    *result =
        (((s[1] - '0') * (8 * 8)) + ((s[2] - '0') * (8)) + (s[3] - '0'));
    return (T);
}

static BOOL config_parse_time(unsigned long *result, char *text)
{
    char *s;
    unsigned long multiple;

    if (!config_parse_rest(&text))
        return (NIL);

    for (s = text; s[1]; s++)
        if (!Uisdigit(*s))
            return (NIL);

    if (Uisdigit(*s))
        multiple = 1;
    else {
        switch (Utoupper(*s)) {
        case 'S':
            multiple = 1;
            break;
        case 'M':
            multiple = 60;
            break;
        case 'H':
            multiple = 60 * 60;
            break;
        case 'D':
            multiple = 24 * 60 * 60;
            break;
        default:
            return (NIL);
        }
        *s++ = '\0';
        if (*s)
            return (NIL);
    }

    *result = (multiple * atoi(text));
    return (T);
}

static BOOL
config_parse_string(char **result, char *text, struct pool *pool)
{
    if (!config_parse_rest(&text))
        return (NIL);

    text = string_trim_whitespace(text);

    *result = pool_strdup(pool, string_trim_whitespace(text));
    return (T);
}

static BOOL
config_parse_list(struct list **listp, char *text, struct pool *pool)
{
    char *next;

    if (!config_parse_rest(&text))
        return (NIL);

    *listp = list_create(pool, T);

    while ((next = strchr(text, ':'))) {
        *next++ = '\0';
        text = string_trim_whitespace(text);

        if (text && text[0])
            list_push(*listp, NIL, text);

        text = next;
    }

    if (text && text[0])
        text = string_trim_whitespace(text);

    if (text && text[0])
        list_push(*listp, NIL, text);

    return (T);
}

/* ====================================================================== */

/* config_parse_single() *************************************************
 *
 * Parse a single (key, value) pair extracted from configuration file
 * or passed as command line option.
 *   config:
 *      key: Text for key
 *    value: Text for value
 *   lineno: Line number in configuration file (for error messages)
 *           0 if this option passed in on the command line.
 ************************************************************************/

static BOOL
config_parse_single(struct config *config, char *key, char *value,
                    unsigned long lineno)
{
    struct pool *pool = config->pool;
    int i;
    char *ptr;

    if ((i = config_find_option(key)) < 0) {
        if (lineno > 0L)
            config_paniclog(config,
                            ("Error parsing configuration file at line %lu: "
                             "Unknown option %s"), lineno, key);
        else
            config_paniclog(config, "Unknown option %s", key);
        return (NIL);
    }

    ptr = ((char *) config) + config_options[i].offset;

    switch (config_options[i].type) {
    case config_bool:
        if (!config_parse_bool((BOOL *) ptr, value)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes boolean argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes boolean argument",
                                key);
            return (NIL);
        }
        break;
    case config_number:
        if (!config_parse_number((unsigned long *) ptr, value)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes a numeric argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes a numeric argument",
                                key);
            return (NIL);
        }
        break;
    case config_perm:
        if (!config_parse_perm((unsigned long *) ptr, value)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes a permission argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes a permission argument",
                                key);
            return (NIL);
        }
        break;
    case config_time:
        if (!config_parse_time((unsigned long *) ptr, value)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes a time argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes a time argument",
                                key);
            return (NIL);
        }
        break;
    case config_string:
    case config_path:
        if (!config_parse_string((char **) ptr, value, pool)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes string argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes string argument",
                                key);
            return (NIL);
        }
        break;
    case config_list:
        if (!config_parse_list((struct list **) ptr, value, pool)) {
            if (lineno > 0L)
                config_paniclog(config,
                                ("Error parsing configuration file at line %lu: "
                                 "Option %s takes a time argument"),
                                lineno, key);
            else
                config_paniclog(config,
                                "option \"%s\" takes a list argument",
                                key);
            return (NIL);
        }
        break;
    default:
        return (NIL);
    }
    return (T);
}

/* ====================================================================== */

/* config_skip_whitespace() **********************************************
 *
 * Skip over whitespace in string (clone of string_skip_whitespace?)
 *     sp:  Ptr to string. Updated to reflect location of next token
 *
 * Returns: Location of the next token.
 ************************************************************************/

static char *config_skip_whitespace(char **sp)
{
    char *s = *sp;

    if (!s)
        return (NIL);

    /* Skip leading whitespace */
    while ((*s == ' ') || (*s == '\t'))
        s++;

    *sp = s;

    return ((*s) ? s : NIL);
}

/* config_get_key() *******************************************************
 *
 * Get key token from config line (token terminated by whitespace or '='
 * character).
 *     sp:  Ptr to string. Updated to reflect location of next token
 *
 * Returns: Location of the next token.
 *************************************************************************/

static char *config_get_key(char **sp)
{
    char *s = *sp, *result;

    if (!s)
        return (NIL);

    while ((*s == ' ') || (*s == '\t'))
        s++;

    /* Record position of this token */
    result = s;

    /* Find next whitespace character, '=', or end of string */
    while ((*s) && (*s != ' ') && (*s != '\t') && (*s != '='))
        s++;

    /* Tie off the string unless \0 already reached, or '=' separator */
    if (*s && (*s != '=')) {
        *s++ = '\0';

        while ((*s == ' ') || (*s == '\t'))
            s++;
    }

    if (*s != '=')
        return (NIL);

    *s++ = '\0';

    while ((*s == ' ') || (*s == '\t'))
        s++;

    /* Record position of first non-whitespace character for next caller */
    *sp = s;

    return (result);
}

/* config_check_keyword() ************************************************
 *
 * Check whether string at (*sp) matches given keyword,
 *     sp:  Ptr to string. Updated to reflect location of next token
 *
 * Returns: T if substring match suceeded.
 ************************************************************************/

static BOOL config_check_keyword(char **sp, char *keyword)
{
    char *s = *sp;
    unsigned long len = strlen(keyword);

    if (strncmp(s, keyword, len) != 0)
        return (NIL);

    s += len;

    /* Keyword must be followed by whitespace */
    if ((*s != ' ') && (*s != '\t'))
        return (NIL);

    while ((*s == ' ') || (*s == '\t'))
        s++;

    /* Record position of first non-whitespace character for next caller */
    *sp = s;

    return (T);
}

/* ====================================================================== */

/* config_parse_local_domain() *******************************************
 *
 * Parse a "local_domain" line
 *    config:
 *    lineno:
 *    option: local_domain option to parse
 ************************************************************************/

static BOOL
config_parse_local_domain(struct config *config,
                          unsigned long lineno, char *option)
{
    struct pool *pool = config->pool;
    struct list **listp = &config->local_domain_list;
    struct config_local_domain *ld;
    char *domain;
    char *file;

    if ((domain = string_get_token(&option)) == NIL) {
        if (lineno > 0) {
            config_paniclog(config,
                            ("Line %lu of config file badly formatted:"
                             " \"local_domain %s %s\""), lineno, domain,
                            option);

        } else {
            config_paniclog(config,
                            "Option badly formatted: \"local_domain %s %s\"",
                            domain, option);
        }
        return (NIL);
    }
    file = string_get_token(&option);

    domain = string_trim_whitespace(domain);
    if (file)
        file = string_trim_whitespace(file);

    ld = pool_alloc(pool, sizeof(struct config_local_domain));
    ld->file = (file) ? pool_strdup(pool, file) : NIL;
    ld->cdb_map = NIL;

    if (*listp == NIL)
        *listp = list_create(pool, T);

    list_push(*listp, (struct list_item *) ld, domain);
    return (T);
}

/* config_parse_ispell_language() ****************************************
 *
 * Parse an "ispell_language" line
 *    config:
 *    lineno:
 *    option: local_domain option to parse
 ************************************************************************/

static BOOL
config_parse_ispell_language(struct config *config,
                             unsigned long lineno, char *option)
{
    struct pool *pool = config->pool;
    struct list **listp = &config->ispell_language_list;
    struct config_language *lang;
    char *name;
    char *desc;

    if ((name = string_get_token(&option)) == NIL) {
        if (lineno > 0)
            config_paniclog(config,
                            "Empty \"use_ispell_language\" clause at line %lu",
                            lineno);
        else
            config_paniclog(config,
                            "Empty \"use_ispell_language\" clause");
        return (NIL);
    }
    if (!config_parse_string(&desc, option, pool)) {
        if (lineno > 0)
            config_paniclog(config,
                            "Invalid \"use_ispell_language\" clause at line %lu",
                            lineno);
        else
            config_paniclog(config,
                            "Invalid \"use_ispell_language\" clause");
        return (NIL);
    }

    lang = pool_alloc(pool, sizeof(struct config_language));
    lang->name = pool_strdup(pool, name);
    lang->desc = desc;

    if (*listp == NIL)
        *listp = list_create(pool, T);

    list_push(*listp, (struct list_item *) lang, name);
    return (T);
}

/* config_parse_template() ***********************************************
 *
 * Parse a "template" line
 *    config:
 *    lineno:
 *    option: templaye option to parse
 ************************************************************************/

static BOOL
config_parse_template(struct config *config, unsigned long lineno,
                      char *option)
{
    struct pool *pool = config->pool;
    struct list **listp = &config->template_list;
    struct config_template *template;
    char *name;
    char *desc;

    if ((name = string_get_token(&option)) == NIL) {
        if (lineno > 0)
            config_paniclog(config,
                            "Empty \"template\" clause at line %lu",
                            lineno);
        else
            config_paniclog(config,
                            "Empty \"template\" clause");
        return (NIL);
    }
    if (!config_parse_string(&desc, option, pool)) {
        if (lineno > 0)
            config_paniclog(config,
                            "Invalid \"template\" clause at line %lu",
                            lineno);
        else
            config_paniclog(config,
                            "Invalid \"template\" clause");
        return (NIL);
    }

    template = pool_alloc(pool, sizeof(struct config_template));
    template->name = pool_strdup(pool, name);
    template->description = desc;

    if (*listp == NIL)
        *listp = list_create(pool, T);

    list_push(*listp, (struct list_item *) template, name);
    return (T);
}

/* config_parse_theme() **************************************************
 *
 * Parse an "theme" line
 *    config:
 *    lineno:
 *    option: theme option to parse
 ************************************************************************/

static BOOL
config_parse_theme(struct config *config, unsigned long lineno,
                   char *option)
{
    struct pool *pool = config->pool;
    struct config_theme *theme;
    char *name;
    char *key;
    char *value;

    if ((name = string_get_token(&option)) == NIL) {
        if (lineno > 0)
            config_paniclog(config, "Empty theme clause at line %lu",
                            lineno);
        else
            config_paniclog(config, "Empty theme clause");
        return (NIL);
    }

    if ((key = string_get_token(&option)) == NIL) {
        if (lineno > 0)
            config_paniclog(config,
                            "Incomplete theme clause at line %lu", lineno);
        else
            config_paniclog(config, "Incomplete theme clause");
        return (NIL);
    }

    if (!config_parse_string(&value, option, pool)) {
        if (lineno > 0)
            config_paniclog(config, "Invalid theme clause at line %lu",
                            lineno);
        else
            config_paniclog(config, "Invalid theme clause");
        return (NIL);
    }

    /* Create new theme list if needed */
    if (config->theme_list == NIL)
        config->theme_list = list_create(pool, T);

    /* Create new theme if required */
    if ((theme = (struct config_theme *)
         list_lookup_byname(config->theme_list, name)) == NIL) {
        theme = config_theme_create(pool);
        list_push(config->theme_list, (struct list_item *) theme, name);
    }

    if (!strcmp(key, "description"))
        theme->description = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor"))
        theme->fgcolor = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor_link"))
        theme->fgcolor_link = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor_quote1"))
        theme->fgcolor_quote1 = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor_quote2"))
        theme->fgcolor_quote2 = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor_quote3"))
        theme->fgcolor_quote3 = pool_strdup(pool, value);
    else if (!strcmp(key, "fgcolor_quote4"))
        theme->fgcolor_quote4 = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor"))
        theme->bgcolor = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor_banner"))
        theme->bgcolor_banner = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor_row1"))
        theme->bgcolor_row1 = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor_row2"))
        theme->bgcolor_row2 = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor_status"))
        theme->bgcolor_status = pool_strdup(pool, value);
    else if (!strcmp(key, "bgcolor_status_none"))
        theme->bgcolor_status_none = pool_strdup(pool, value);
    else {
        if (lineno > 0)
            config_paniclog(config,
                            "Unknown theme key \"%s\" at line %lu of config file",
                            key, lineno);
        else
            config_paniclog(config, "Unknown theme key \"%s\"", key);

        return (NIL);
    }

    return (T);
}

/* config_parse_http_port() ***********************************************
 *
 * Parse a "http_port" line
 *    config:
 *    lineno:
 *   use_ssl: This is a HTTPS port
 *    option: local_domain option to parse
 ************************************************************************/

static BOOL
config_parse_http_port(struct config *config, BOOL use_ssl,
                       unsigned long lineno, char *orig_option)
{
    struct pool *pool = config->pool;
    struct list **listp = &config->http_port_list;
    char *s;
    char *option = pool_strdup(pool, orig_option);
    char *interface;
    unsigned long port;
    BOOL error;
    struct config_http_port *chp;

    interface = NIL;
    port = 0;
    error = NIL;

    if (!config_parse_rest(&option))
        error = T;
    else if ((s = (strrchr(option, ':')))) {
        *s++ = '\0';
        if (!config_parse_number(&port, s))
            error = T;
        interface = pool_strdup(pool, option);
    } else if (!config_parse_number(&port, option))
        error = T;

    if (error) {
        if (lineno > 0) {
            config_paniclog(config,
                            ("Line %lu of config file badly formatted:"
                             " \"use_http%s_port %lu %s\""),
                            lineno, ((use_ssl) ? "s" : ""), port,
                            orig_option);
        } else {
            config_paniclog(config,
                            "Option badly formatted: \"use_http%s_port %lu %s\"",
                            ((use_ssl) ? "s" : ""), port, orig_option);
        }
        return (NIL);
    }

    chp = pool_alloc(pool, sizeof(struct config_http_port));

    chp->interface = interface;
    chp->port = port;
    chp->use_ssl = use_ssl;

    if (*listp == NIL)
        *listp = list_create(pool, T);

    list_push(*listp, (struct list_item *) chp, NIL);
    return (T);
}

/* ====================================================================== */

/* config_parse_option() *************************************************
 *
 * Parse a single option passed in from the command line
 *   config:
 *   option: "key=value" or "<special> <args>" to parse
 *
 * Returns: T if option parsed successfully
 *          NIL otherwise (message will be sent to paniclog).
 ************************************************************************/

BOOL config_parse_option(struct config * config, char *option)
{
    char *key, *value;

    option = string_trim_whitespace(option);

    if (option[0] == '\0')
        return (T);

    if (config_check_keyword(&option, "local_domain"))
        return (config_parse_local_domain(config, 0L, option));

    if (config_check_keyword(&option, "use_ispell_language"))
        return (config_parse_ispell_language(config, 0L, option));

    if (config_check_keyword(&option, "template"))
        return (config_parse_template(config, 0L, option));

    if (config_check_keyword(&option, "theme"))
        return (config_parse_theme(config, 0L, option));

    if (config_check_keyword(&option, "use_http_port"))
        return (config_parse_http_port(config, NIL, 0L, option));

    if (config_check_keyword(&option, "use_https_port"))
        return (config_parse_http_port(config, T, 0L, option));

    if ((key = config_get_key(&option)) == NIL) {
        config_paniclog(config, "Option badly formatted: \"%s\"", option);
        return (NIL);
    }

    if ((value = config_skip_whitespace(&option)) == NIL) {
        config_paniclog(config,
                        "Option badly formatted: \"%s %s\"", key, option);
        return (NIL);
    }

    return (config_parse_single(config, key, value, 0L));
}

/* ====================================================================== */

/* config_get_line() ****************************************************
 *
 * Get a linear whitespace line (e.g: folded RFC822 or HTTP header line)
 *       sp: Ptr to current string location
 *           (updated to point to following line)
 *  squeeze: Remove superfluous spaces from result.
 *
 * Returns: Ptr to this line
 ***********************************************************************/

static char *config_get_line(char **sp, BOOL squeeze)
{
    char *s, *result;

    s = *sp;

    if (!(s && s[0]))
        return (NIL);

    /* CR, LF or CRLF before data proper starts? */
    if ((s[0] == '\015') || (s[0] == '\012')) {
        result = s;
        if ((s[0] == '\015') && (s[1] == '\012')) {     /* CRLF */
            *s = '\0';
            s += 2;
        } else
            *s++ = '\0';        /* CR or LF */

        *sp = s;
        return (result);
    }

    result = s;                 /* Record position of non-LWS */

    while (*s) {
        if ((*s == '\015') || (*s == '\012')) { /* CR, LF or CRLF */
            char *t = s;

            s += ((s[0] == '\015') && (s[1] == '\012')) ? 2 : 1;

            if ((*s != ' ') && (*s != '\t')) {
                *t = '\0';
                break;
            }
        } else
            s++;
    }
    *sp = s;                    /* Set up for next caller */
    return (result);
}

/* config_count_line() ***************************************************
 *
 * Count number of lines in string
 ************************************************************************/

static unsigned long config_count_line(char *s)
{
    unsigned long lines = 1;

    while (*s) {
        if ((*s == '\015') || (*s == '\012')) {
            s += ((s[0] == '\015') && (s[1] == '\012')) ? 2 : 1;
            lines++;
        } else
            s++;
    }
    return (lines);
}


/* config_compress_line() ************************************************
 *
 * Remove continuation sequences from LWS line
 ************************************************************************/

static BOOL config_compress_line(char *s)
{
    char *t = s;

    while (*s) {
        /* Check for correctly folded line */
        if ((s[0] == '\\') && ((s[1] == '\015') || (s[1] == '\012'))) {
            s += ((s[1] == '\015') && (s[2] == '\012')) ? 3 : 2;

            while (string_isspace(*s))
                s++;

            continue;
        }

        /* Check for unexpected line break */
        if ((s[0] == '\015') || (s[0] == '\012'))
            return (NIL);

        if (s > t)
            *t++ = *s++;
        else
            t++, s++;
    }
    *t = '\0';

    return (T);
}

/* ====================================================================== */


/* config_parse_file() ***************************************************
 *
 * Parse configuration file
 *     config:
 *   filename: Name of configuration file (main() routine will pass
 *             compile time default or user specified version).
 *
 * Returns: T if option parsed successfully
 *          NIL otherwise (message will be sent to paniclog).
 ************************************************************************/

BOOL config_parse_file(struct config * config, char *filename)
{
    struct pool *tpool = pool_create(CONFIG_PREFERRED_TEMP_POOL_SIZE);
    struct buffer *b =
        buffer_create(tpool, CONFIG_PREFERRED_TEMP_POOL_SIZE);
    char *text;
    char *line, *key, *value;
    unsigned long next_lineno = 1;
    unsigned long cur_lineno = 1;
    BOOL okay;
    FILE *file;
    int c;

    /* Read configuration file into buffer, realloc as char array */
    if ((file = fopen(filename, "r")) == NULL) {
        config_paniclog(config,
                        "Couldn't open configuration file: \"%s\"",
                        filename);
        return (NIL);
    }
    while ((c = getc(file)) != EOF)
        bputc(b, c);
    fclose(file);

    text = buffer_fetch(b, 0, buffer_size(b), NIL);

    /* Prep options array if we need to */
    if ((options_size == 0L) && !config_init()) {
        pool_free(tpool);
        return (NIL);
    }

    okay = T;
    while ((line = config_get_line(&text, NIL))) {
        cur_lineno = next_lineno;
        next_lineno += config_count_line(line);

        if (!config_compress_line(line)) {
            config_paniclog(config,
                            "Invalid folded line starting at line %d: \"%s\"",
                            cur_lineno, line);
            okay = NIL;
            continue;
        }

        if (line[0] == '#')
            continue;

        line = string_trim_whitespace(line);

        if (line[0] == '\0')
            continue;

        if (config_check_keyword(&line, "local_domain")) {
            if (!config_parse_local_domain(config, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if (config_check_keyword(&line, "use_ispell_language")) {
            if (!config_parse_ispell_language(config, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if (config_check_keyword(&line, "template")) {
            if (!config_parse_template(config, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if (config_check_keyword(&line, "theme")) {
            if (!config_parse_theme(config, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if (config_check_keyword(&line, "use_http_port")) {
            if (!config_parse_http_port(config, NIL, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if (config_check_keyword(&line, "use_https_port")) {
            if (!config_parse_http_port(config, T, cur_lineno, line))
                okay = NIL;
            continue;
        }

        if ((key = config_get_key(&line)) == NIL) {
            config_paniclog(config,
                            "Line %lu of config file badly formatted: \"%s\"",
                            cur_lineno, line);
            okay = NIL;
            continue;
        }

        if ((value = config_skip_whitespace(&line)) == NIL) {
            config_paniclog(config,
                            "Line %lu of config file badly formatted: \"%s %s\"",
                            cur_lineno, key, line);
            okay = NIL;
            continue;
        }

        if (!config_parse_single(config, key, value, cur_lineno))
            okay = NIL;
    }
    pool_free(tpool);
    return (okay);
}

/* ====================================================================== */

/* Couple of support macros to help test the integrity of the configuration
 * that we have just parsed */

#define TEST_STRING(x, y)                               \
{                                                       \
  if (!(x && x[0])) {                                   \
    config_paniclog(config, "config: "y" not defined"); \
    return(NIL);                                        \
  }                                                     \
}

#define TEST_NUMBER(x, y)                               \
{                                                       \
  if (x == 0) {                                         \
    config_paniclog(config, "config: "y" not defined"); \
    return(NIL);                                        \
  }                                                     \
}

#define HOSTNAME_TMP_BUFFER_SIZE  (8192)

/* ====================================================================== */

/* config_locate_port() **************************************************
 *
 * Lookup HTTP port in config structure
 *    list:
 ************************************************************************/

static struct config_http_port *config_locate_port(struct config *config,
                                                   unsigned long port)
{
    struct list *list = config->http_port_list;
    struct list_item *li;

    for (li = list->head; li; li = li->next) {
        struct config_http_port *chp = (struct config_http_port *) li;

        if (chp->port == port)
            return (chp);
    }
    return (NIL);
}

/* config_find_port() ****************************************************
 *
 * Pick a default HTTP or HTTPS  port
 *    config:
 *  want_ssl:  T   => want a SSL port       (443 preferred)
 *             NIL => want a plaintext port (80 preferred)
 *
 * Returns: Port structure, NIL if none appropriate.
 ************************************************************************/

static struct config_http_port *config_find_port(struct config *config,
                                                 BOOL want_ssl)
{
    struct list *list = config->http_port_list;
    struct list_item *li;

    /* Assign canonical port if it is available */
    for (li = list->head; li; li = li->next) {
        struct config_http_port *chp = (struct config_http_port *) li;

        if (want_ssl) {
            if ((chp->port == 443) && (chp->use_ssl == T))
                return (chp);
        } else {
            if ((chp->port == 80) && (chp->use_ssl == NIL))
                return (chp);
        }
    }

    /* Otherwise scan for first available port of the correct type */
    for (li = list->head; li; li = li->next) {
        struct config_http_port *chp = (struct config_http_port *) li;

        if (want_ssl) {
            if (chp->use_ssl == T)
                return (chp);
        } else {
            if (chp->use_ssl == NIL)
                return (chp);
        }
    }
    return (NIL);
}

/* ====================================================================== */

/* Static support routines for config_check */

static BOOL config_check_sort_mode(char *mode)
{
    if (mode == NIL)
        return (NIL);

    switch (Utoupper(mode[0])) {
    case 'A':
        if (!strcasecmp(mode, "ARRIVAL"))
            return (T);
        break;
    case 'C':
        if (!strcasecmp(mode, "CC"))
            return (T);
        break;
    case 'D':
        if (!strcasecmp(mode, "DATE"))
            return (T);
        break;
    case 'F':
        if (!strcasecmp(mode, "FROM"))
            return (T);
        break;
    case 'S':
        if (!strcasecmp(mode, "SUBJECT"))
            return (T);
        else if (!strcasecmp(mode, "SIZE"))
            return (T);
        break;
    case 'T':
        if (!strcasecmp(mode, "TO"))
            return (T);
        break;
    }

    return (NIL);
}

/* ====================================================================== */

/* config_check() *********************************************************
 *
 * Check that a parsed configuration is sane and self-consistent.
 *   config:
 *
 * Returns: T if config okay, NIL otherwise
 *************************************************************************/

BOOL config_check(struct config * config)
{
    BOOL use_ssl = NIL;
    struct list_item *li;
    struct config_http_port *chp;
    char hostname[MAX_ADDRESS+1], domainname[MAX_ADDRESS+1];

    TEST_STRING(config->icon_dir, "icon_dir");
    TEST_STRING(config->socket_dir, "socket_dir");
    TEST_STRING(config->init_socket_name, "init_socket_name");
    TEST_STRING(config->ssl_session_dir, "ssl_session_dir");
#ifndef MUTEX_SEMAPHORE
    TEST_STRING(config->lock_dir, "lock_dir");
#endif
    TEST_STRING(config->log_dir, "log_dir");
    TEST_STRING(config->tmp_dir, "tmp_dir");
    TEST_STRING(config->pid_dir, "pid_dir");
    TEST_STRING(config->bin_dir, "bin_dir");

    if (config->prayer_user && config->prayer_uid) {
        config_paniclog(config,
                        "config: prayer_user and prayer_uid both defined");
        return (NIL);
    }

    if (config->prayer_group && config->prayer_gid) {
        config_paniclog(config,
                        "config: prayer_group and prayer_gid both defined");
        return (NIL);
    }

    /* Calculate hostnames */
    getdomainnames(hostname, sizeof(hostname)-1,
		   domainname, sizeof(domainname)-1);

    if (!(config->hostname_canonical && config->hostname_canonical[0]))
        config->hostname_canonical = pool_strdup(config->pool, hostname);

    /* Set hostname from environment variable? */
    if (getenv("PRAYER_HOSTNAME"))
        config->hostname =
            pool_strdup(config->pool, getenv("PRAYER_HOSTNAME"));

    if (config->hostname && config->hostname[0]) {
        /* Check for __UNDEFINED__ special case */
        if (!strcasecmp(config->hostname, "__UNDEFINED__")) {
            config_paniclog(config,
                            ("config: hostname undefined. Should have been "
                             "set up using --config-option \"hostname=value\"?"));
            return (NIL);
        }
    } else
        config->hostname = pool_strdup(config->pool, hostname);

    TEST_NUMBER(config->http_timeout_idle, "http_timeout_idle");
    TEST_NUMBER(config->http_timeout_icons, "http_timeout_icons");
    TEST_NUMBER(config->http_timeout_session, "http_timeout_session");

    TEST_NUMBER(config->http_max_method_size, "http_max_method_size");
    TEST_NUMBER(config->http_max_hdr_size, "http_max_hdr_size");
    TEST_NUMBER(config->http_max_body_size, "http_max_body_size");

    if (!(config->http_port_list && config->http_port_list->head)) {
        config_paniclog(config, "config: No HTTP or HTTPS ports defined");
        return (NIL);
    }

    /* Check ssl_default_port, derive automatically if not defined */
    if (config->ssl_default_port) {
        chp = config_locate_port(config, config->ssl_default_port);
        if (chp == NIL) {
            config_paniclog(config,
                            "config: ssl_default_port defines unused port: %lu",
                            config->ssl_default_port);
            return (NIL);
        }
        if (chp->use_ssl == NIL) {
            config_paniclog(config,
                            "config: ssl_default_port defines plaintext port: %lu",
                            config->ssl_default_port);
            return (NIL);
        }
    } else if ((chp = config_find_port(config, T)))
        config->ssl_default_port = chp->port;

    for (li = config->http_port_list->head; li; li = li->next) {
        struct config_http_port *chp = (struct config_http_port *) li;

        if (chp->use_ssl) {
            use_ssl = T;
            break;
        }
    }

    if (use_ssl) {
        TEST_STRING(config->ssl_cert_file, "ssl_cert_file");
        TEST_STRING(config->ssl_privatekey_file, "ssl_privatekey_file");
        TEST_NUMBER(config->ssl_rsakey_lifespan, "ssl_rsakey_lifespan");
    }

    if (config->direct_enable) {
        if (((config->direct_ssl_first == 0)
             || (config->direct_ssl_count == 0))
            && ((config->direct_plain_first == 0)
                || (config->direct_plain_count == 0))) {
            config_paniclog(config, ("config: direct_enable set, "
                                     "but no direct access ports have been set up"));
            return (NIL);
        }

        /* Check for overlap */
        if (config->direct_ssl_first && config->direct_ssl_count &&
            config->direct_ssl_first && config->direct_ssl_count) {
            unsigned long start1 = config->direct_ssl_first;
            unsigned long len1 = config->direct_ssl_count;
            unsigned long start2 = config->direct_plain_first;
            unsigned long len2 = config->direct_plain_count;

            if ((len1 > 0) && (len2 > 0)) {
                if (((start1 + len1) > start2)
                    && (start1 + len1) <= (start2 + len2)) {
                    config_paniclog(config, ("config: DIRECT mode SSL and "
                                             "plaintext ports overlap"));
                    return (NIL);
                }

                if (((start2 + len2) > start1)
                    && (start2 + len2) <= (start1 + len1)) {
                    config_paniclog(config, ("config: DIRECT mode SSL and "
                                             "plaintext ports overlap"));

                    return (NIL);
                }
            }
        }
    }

    TEST_NUMBER(config->session_timeout, "session_timeout");

    if (config->session_idle_time >= config->session_timeout) {
        config_paniclog(config,
                        "config: session_idle_time > session_timeout_compose");
        return (NIL);
    }

    if (config->session_timeout_compose > 0) {
        if (config->session_idle_time >= config->session_timeout_compose) {
            config_paniclog(config,
                            "config: session_idle_time > session_timeout_compose");
            return (NIL);
        }
    }

    TEST_NUMBER(config->icon_expire_timeout, "icon_expire_timeout");
    TEST_NUMBER(config->static_expire_timeout, "static_expire_timeout");
    TEST_NUMBER(config->session_key_len, "session_key_len");
    TEST_NUMBER(config->list_addr_maxlen, "list_addr_maxlen");
    TEST_NUMBER(config->list_subject_maxlen, "list_subject_maxlen");

    TEST_STRING(config->prefs_folder_name, "prefs_folder_name");
    TEST_STRING(config->sendmail_path, "sendmail_path");

    /* Default user preferences */
    TEST_STRING(config->sort_mode, "sort_mode");
    TEST_STRING(config->postponed_folder, "postponed_folder");
    TEST_STRING(config->sent_mail_folder, "sent_mail_folder");
    TEST_STRING(config->ispell_language, "ispell_language");
    TEST_NUMBER(config->small_rows, "small_rows");
    TEST_NUMBER(config->small_cols, "small_cols");
    TEST_NUMBER(config->large_rows, "large_rows");
    TEST_NUMBER(config->large_cols, "large_cols");


    if (!config_check_sort_mode(config->sort_mode)) {
        config_paniclog(config, "config: sort_mode \"%s\" is invalid",
                        config->sort_mode);
        return (NIL);
    }

    return (T);
}

/* ====================================================================== */

/* Static utility routine for config_expand() */

static BOOL
config_macro_expand(struct config *config, char *name, char **valuep)
{
    struct pool *pool = config->pool;
    char *value;

    if (*valuep == NIL)
        return (NIL);

    value = *valuep;

    if (!strncmp(value, "$prefix", strlen("$prefix"))) {
        if (config->prefix == NIL) {
            config_paniclog(config,
                            "$prefix referenced by \"%s\" but not defined",
                            name);
            return (NIL);
        }
        *valuep =
            pool_strcat(pool, config->prefix, value + strlen("$prefix"));
        return (T);
    }

    if (!strncmp(value, "${prefix}", strlen("${prefix}"))) {
        if (config->prefix == NIL) {
            config_paniclog(config,
                            "$prefix referenced by \"%s\" but not defined",
                            name);
            return (NIL);
        }
        *valuep =
            pool_strcat(pool, config->prefix, value + strlen("${prefix}"));
        return (T);
    }

    if (!strncmp(value, "$var_prefix", strlen("$var_prefix"))) {
        if (config->var_prefix == NIL) {
            config_paniclog(config,
                            "$var_prefix referenced by \"%s\" but not defined",
                            name);
            return (NIL);
        }
        *valuep =
            pool_strcat(pool, config->var_prefix,
                        value + strlen("$var_prefix"));
        return (T);
    }

    if (!strncmp(value, "${var_prefix}", strlen("${var_prefix}"))) {
        if (config->var_prefix == NIL) {
            config_paniclog(config,
                            "$var_prefix referenced by \"%s\" but not defined",
                            name);
            return (NIL);
        }
        *valuep =
            pool_strcat(pool, config->var_prefix,
                        value + strlen("${var_prefix}"));
        return (T);
    }

    return (T);
}

/* ====================================================================== */

/* config_expand() ********************************************************
 *
 * Expand macros from configuration file.
 * Handles: $prefix and $var_prefix. UID and GID lookups for prayer_user
 *          and prayer_group.
 *   config:
 *
 * Returns: T if config expansion okay, NIL otherwise
 *************************************************************************/

BOOL config_expand(struct config * config)
{
    unsigned long offset;
    BOOL okay = T;
    struct passwd *pwd;
    struct group *group;
    char cwd[CONFIG_CWD_MAXLEN];

    if (getcwd(cwd, CONFIG_CWD_MAXLEN) == NIL) {
        config_paniclog(config, "config_expand(): getcwd() failed");
        return (NIL);
    }

    for (offset = 0L; offset < options_size; offset++) {
        char *ptr = ((char *) config) + config_options[offset].offset;
        char **valuep = (char **) ptr;

        if (config_options[offset].type != config_path)
            continue;

        if (*valuep == NIL)
            continue;

        if (*valuep[0] == '\0')
            continue;

        if (!config_macro_expand
            (config, config_options[offset].name, valuep))
            okay = NIL;

        if (*valuep && (*valuep[0] != '/'))
            *valuep = pool_strcat3(config->pool, cwd, "/", *valuep);
    }

    if (config->prayer_user) {
        if ((pwd = getpwnam(config->prayer_user))) {
            config->prayer_uid = pwd->pw_uid;
        } else {
            config_paniclog(config,
                            "prayer_user: User \"%s\" not defined",
                            config->prayer_group);
            okay = NIL;
        }
    }

    if (config->prayer_group) {
        if ((group = getgrnam(config->prayer_group))) {
            config->prayer_gid = group->gr_gid;
        } else {
            config_paniclog(config,
                            "prayer_group: Group \"%s\" not defined",
                            config->prayer_group);
            okay = NIL;
        }
    }

    if ((config->prayer_uid != 0L) && (config->prayer_uid != 0L)) {
        if (config->directory_perms == 0L)
            config->directory_perms = 0750;

        if (config->file_perms == 0L)
            config->file_perms = 0640;
    } else {
        if (config->directory_perms == 0L)
            config->directory_perms = 0755;

        if (config->file_perms == 0L)
            config->file_perms = 0644;
    }

    if (config->check_directory_perms) {
        if ((config->prayer_uid == 0) || (config->prayer_gid == 0)) {
            config_paniclog(config, ("check_directory_perms set,"
                                     " but user and group not supplied"));
            return (NIL);
        }
    }

    /* Fill in default_domain, return_path_domain and local_domains from
     * hostname if nothing else supplied */
    if (config->default_domain == NIL)
        config->default_domain =
            pool_strdup(config->pool, config->hostname);

    if (config->return_path_domain == NIL)
        config->return_path_domain
            = pool_strdup(config->pool, config->default_domain);

    if (config->filter_domain_pattern == NIL)
        config->filter_domain_pattern
            = pool_strdup(config->pool, config->default_domain);

    if (config->local_domain_list) {
        struct list_item *li;

        for (li = config->local_domain_list->head; li; li = li->next) {
            struct config_local_domain *cld =
                (struct config_local_domain *) li;

            if (cld->file) {
                if (!config_macro_expand
                    (config, "local_domain", &cld->file))
                    okay = NIL;
            }
        }
    } else {
        struct config_local_domain *ld;

        ld = pool_alloc(config->pool, sizeof(struct config_local_domain));
        ld->file = NIL;
        ld->cdb_map = NIL;

        config->local_domain_list = list_create(config->pool, T);

        list_push(config->local_domain_list, (struct list_item *) ld,
                  config->default_domain);
    }

    if (config->theme_default_main) {
        if (!(config->theme_main = (struct config_theme *)
              list_lookup_byname(config->theme_list,
                                 config->theme_default_main))) {
            config_paniclog(config,
                            "theme_default_main set to undefined theme");
            return (NIL);
        }
    } else
        config->theme_main = config_theme_create(config->pool);

    if (config->theme_default_help) {
        if (!(config->theme_help = (struct config_theme *)
              list_lookup_byname(config->theme_list,
                                 config->theme_default_help))) {
            config_paniclog(config,
                            "theme_default_help set to undefined theme");
            return (NIL);
        }
    } else
        config->theme_help = config_theme_create(config->pool);

    return (okay);
}

/* ====================================================================== */

/* Static utility routine to check directory permissions. */

static BOOL
config_check_perms(struct config *config, char *dir, struct stat *sbufp)
{
    BOOL okay = T;

    if (config->prayer_uid != sbufp->st_uid)
        okay = NIL;
    else if (config->prayer_gid != sbufp->st_gid)
        okay = NIL;
    else if (sbufp->st_mode & S_IXOTH)
        okay = NIL;

    if (!okay) {
        if (config->prayer_user && config->prayer_group)
            log_fatal(("\"%s\" directory must be owned by user \"%s\","
                       " group \"%s\" and not accessible by othe0rs"),
                      dir, config->prayer_user, config->prayer_group);
        else
            log_fatal(("\"%s\" directory must be owned by uid \"%lu\" and"
                       " gid \"%lu\" and not accessible by others"),
                      dir, config->prayer_user, config->prayer_group);
    }

    return (okay);
}

/* Static utility routine to create working directory */

static BOOL
config_mkdir(struct config *config,
             struct pool *tpool, char *prefix, char *dir)
{
    char *partname, *s;
    struct stat sbuf;
    mode_t mkdir_perms = ((config->directory_perms) ?
                          (config->directory_perms) : 0755);

    BOOL check_intermediate = NIL;

    if (!(dir && dir[0])) {
        log_fatal("[config_mkdir()] Empty directory name supplied");
        /* NOTREACHED */
        return (-1);
    }

    if (prefix && prefix[0] && !strncmp(dir, prefix, strlen(prefix))) {
        /* Scan pathname from end of prefix */
        partname = pool_strdup(tpool, dir);
        s = partname + strlen(prefix);
        check_intermediate = T;
        if (*s == '/')
            s++;
    } else {
        /* Start scanning from start of path */
        partname = pool_strdup(tpool, dir);
        s = partname + 1;
        check_intermediate = NIL;
    }

    /* Find first component of path which doesn't exist */
    while ((s = strchr(s, '/'))) {
        *s = '\0';

        if (stat(partname, &sbuf) < 0)
            break;

        if (check_intermediate && config->check_directory_perms) {
            if (!config_check_perms(config, partname, &sbuf))
                return (NIL);
        } else if (sbuf.st_mode & S_IWOTH) {
            log_fatal(("Problem with directory \"%s\", parent directory \"%s\"" " is world writable!"), dir, partname);
            /* NOTREACHED */
            return (-1);

        }

        *s++ = '/';
    }

    if (s != NIL) {
        if (errno != ENOENT) {
            log_fatal("Couldn't access directory: \"%s\": %s",
                      partname, strerror(errno));
            /* NOTREACHED */
            return (-1);
        }

        /* Try and create each directory in turn */
        while (1) {
            if (mkdir(partname, mkdir_perms) < 0) {
                log_fatal("Couldn't create directory: \"%s\" - \"%s\": %s",
                          dir, partname, strerror(errno));
                /* NOTREACHED */
                return (-1);
            }

            *s++ = '/';
            if (!(s = strchr(s, '/')))
                break;

            *s = '\0';
        }
    }

    /* Does last component in path already exist? */
    if (stat(dir, &sbuf) == 0) {
        if (config->check_directory_perms) {
            if (!config_check_perms(config, dir, &sbuf))
                return (NIL);
        }
        return (T);
    }

    if (errno != ENOENT) {
        log_fatal("Couldn't access directory: \"%s\": %s", dir,
                  strerror(errno));
        /* NOTREACHED */
        return (-1);
    }

    /* Try to make last component in path */
    if (mkdir(dir, mkdir_perms) < 0) {
        log_fatal("Couldn't create directory: \"%s\": %s", dir,
                  strerror(errno));
        /* NOTREACHED */
        return (NIL);
    }

    return (T);
}

/* ====================================================================== */

/* config_mkdirs() ********************************************************
 *
 * Create working directories (typically under ${var_prefix} if they
 * don't already exist. Check permissions on intermediate directories
 * under ${var_prefix} if so configured.
 *************************************************************************/

BOOL config_mkdirs(struct config * config)
{
    struct pool *tpool = pool_create(1024);
    char *var_prefix;
    struct stat sbuf;

    if (config->check_directory_perms && config->var_prefix) {
        if (stat(config->var_prefix, &sbuf) < 0) {
            log_fatal("Directory \"%s\" cannot be accessed: %s",
                      config->var_prefix, strerror(errno));
            /* NOT REACHED */
            return (NIL);
        }
#if 0 /* XXX DPC 10/04/2005: Temporary */
        if (!config_check_perms(config, config->var_prefix, &sbuf))
            return (NIL);
#endif
    }

    var_prefix = (config->var_prefix) ? config->var_prefix : "";

    if (config->log_dir && config->log_dir[0])
        config_mkdir(config, tpool, var_prefix, config->log_dir);

    if (config->lock_dir && config->lock_dir[0])
        config_mkdir(config, tpool, var_prefix, config->lock_dir);

    if (config->socket_dir && config->socket_dir[0])
        config_mkdir(config, tpool, var_prefix, config->socket_dir);

    if (config->socket_split_dir) {
        char *v
            =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

        while (*v)
            config_mkdir(config, tpool, var_prefix,
                         pool_printf(tpool, "%s/%c", config->socket_dir,
                                     *v++));
    }

    if (config->ssl_session_dir && config->ssl_session_dir[0])
        config_mkdir(config, tpool, var_prefix, config->ssl_session_dir);

    if (config->tmp_dir && config->tmp_dir[0])
        config_mkdir(config, tpool, var_prefix, config->tmp_dir);

    if (config->pid_dir && config->pid_dir[0])
        config_mkdir(config, tpool, var_prefix, config->pid_dir);

    if (config->sending_allow_dir && config->sending_allow_dir[0])
        config_mkdir(config, tpool, var_prefix, config->sending_allow_dir);

    if (config->sending_block_dir && config->sending_block_dir[0])
        config_mkdir(config, tpool, var_prefix, config->sending_block_dir);

    pool_free(tpool);
    return (T);
}

void
config_extract_ssl(struct config *src, struct ssl_config *dst)
{
    dst->ssl_cipher_list       = src->ssl_cipher_list;
    dst->ssl_server_preference = src->ssl_server_preference;
    dst->ssl_session_dir      = src->ssl_session_dir;
    dst->ssl_cert_file        = src->ssl_cert_file;
    dst->ssl_privatekey_file  = src->ssl_privatekey_file;
    dst->ssl_dh_file          = src->ssl_dh_file;
    dst->ssl_session_timeout  = src->ssl_session_timeout;
    dst->ssl_rsakey_lifespan  = src->ssl_rsakey_lifespan;
    dst->ssl_rsakey_freshen   = src->ssl_rsakey_freshen;
    dst->ssl_default_port     = src->ssl_default_port;
    dst->egd_socket           = src->egd_socket;
    dst->log_debug            = src->log_debug;
}
