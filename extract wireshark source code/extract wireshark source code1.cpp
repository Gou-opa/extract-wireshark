// extract wireshark source code.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <cstring>
#include <stdio.h>
#include <glib.h>

//#include "capture_file.h"
using namespace std;


typedef enum {
	CF_OK,      /**< operation succeeded */
	CF_ERROR    /**< operation got an error (function may provide err with details) */
} cf_status_t;

typedef struct {
	time_t	secs;
	int	nsecs;
} nstime_t;

struct eth_phdr {
	gint   fcs_len;  /* Number of bytes of FCS - -1 means "unknown" */
};
struct x25_phdr {
	guint8  flags;   /* ENCAP_LAPB, ENCAP_V120 : 1st bit means From DCE */
};

/* Packet "pseudo-header" information for ISDN capture files. */

/* Direction */
struct isdn_phdr {
	gboolean uton;
	guint8   channel;   /* 0 = D-channel; n = B-channel n */
};
struct atm_phdr {
	guint32 flags;      /* status flags */
	guint8  aal;        /* AAL of the traffic */
	guint8  type;       /* traffic type */
	guint8  subtype;    /* traffic subtype */
	guint16 vpi;        /* virtual path identifier */
	guint16 vci;        /* virtual circuit identifier */
	guint8  aal2_cid;   /* channel id */
	guint16 channel;    /* link: 0 for DTE->DCE, 1 for DCE->DTE */
	guint16 cells;      /* number of cells */
	guint16 aal5t_u2u;  /* user-to-user indicator */
	guint16 aal5t_len;  /* length of the packet */
	guint32 aal5t_chksum;   /* checksum for AAL5 packet */
};
#define ASCEND_MAX_STR_LEN 64
struct ascend_phdr {
	guint16 type;                         /* ASCEND_PFX_*, as defined above */
	char    user[ASCEND_MAX_STR_LEN];     /* Username, from wandsession header */
	guint32 sess;                         /* Session number, from wandsession header */
	char    call_num[ASCEND_MAX_STR_LEN]; /* Called number, from WDD header */
	guint32 chunk;                        /* Chunk number, from WDD header */
	guint32 task;                         /* Task number */
};
struct p2p_phdr {
	int sent; /* TRUE=sent, FALSE=received, -1=unknown*/
};
struct ieee_802_11_fhss {
	guint    has_hop_set : 1;
	guint    has_hop_pattern : 1;
	guint    has_hop_index : 1;

	guint8   hop_set;        /* Hop set */
	guint8   hop_pattern;    /* Hop pattern */
	guint8   hop_index;      /* Hop index */
};
struct ieee_802_11g {
	/* Which of this information is present? */
	guint    has_short_preamble : 1;
	guint    has_mode : 1;

	gboolean short_preamble; /* Short preamble */
	guint32  mode;           /* Various proprietary extensions */
};
struct ieee_802_11n {
	/* Which of this information is present? */
	guint    has_mcs_index : 1;
	guint    has_bandwidth : 1;
	guint    has_short_gi : 1;
	guint    has_greenfield : 1;
	guint    has_fec : 1;
	guint    has_stbc_streams : 1;
	guint    has_ness : 1;

	guint16  mcs_index;      /* MCS index */
	guint    bandwidth;      /* Bandwidth = 20 MHz, 40 MHz, etc. */
	guint    short_gi : 1;     /* True for short guard interval */
	guint    greenfield : 1;   /* True for greenfield, short for mixed */
	guint    fec : 1;          /* FEC: 0 = BCC, 1 = LDPC */
	guint    stbc_streams : 2; /* Number of STBC streams */
	guint    ness;           /* Number of extension spatial streams */
};
struct ieee_802_11ac {
	/* Which of this information is present? */
	guint    has_stbc : 1;
	guint    has_txop_ps_not_allowed : 1;
	guint    has_short_gi : 1;
	guint    has_short_gi_nsym_disambig : 1;
	guint    has_ldpc_extra_ofdm_symbol : 1;
	guint    has_beamformed : 1;
	guint    has_bandwidth : 1;
	guint    has_fec : 1;
	guint    has_group_id : 1;
	guint    has_partial_aid : 1;

	guint    stbc : 1;         /* 1 if all spatial streams have STBC */
	guint    txop_ps_not_allowed : 1;
	guint    short_gi : 1;     /* True for short guard interval */
	guint    short_gi_nsym_disambig : 1;
	guint    ldpc_extra_ofdm_symbol : 1;
	guint    beamformed : 1;
	guint8   bandwidth;      /* Bandwidth = 20 MHz, 40 MHz, etc. */
	guint8   mcs[4];         /* MCS index per user */
	guint8   nss[4];         /* NSS per user */
	guint8   fec;            /* Bit array of FEC per user: 0 = BCC, 1 = LDPC */
	guint8   group_id;
	guint16  partial_aid;
};
struct ieee_802_11ad {
	/* Which of this information is present? */
	guint    has_mcs_index : 1;

	guint8   mcs;            /* MCS index */
};
struct ieee_802_11_fhss {
    guint    has_hop_set : 1;
    guint    has_hop_pattern : 1;
    guint    has_hop_index : 1;

    guint8   hop_set;        /* Hop set */
    guint8   hop_pattern;    /* Hop pattern */
    guint8   hop_index;      /* Hop index */
};

/*
 * 802.11b.
 */
struct ieee_802_11b {
    /* Which of this information is present? */
    guint    has_short_preamble:1;

    gboolean short_preamble; /* Short preamble */
};

/*
 * 802.11a.
 */
struct ieee_802_11a {
    /* Which of this information is present? */
    guint    has_channel_type:1;
    guint    has_turbo_type:1;

    guint    channel_type:2;
    guint    turbo_type:2;
};
struct ieee_802_11g {
    /* Which of this information is present? */
    guint    has_short_preamble:1;
    guint    has_mode:1;

    gboolean short_preamble; /* Short preamble */
    guint32  mode;           /* Various proprietary extensions */
};
struct ieee_802_11n {
    /* Which of this information is present? */
    guint    has_mcs_index:1;
    guint    has_bandwidth:1;
    guint    has_short_gi:1;
    guint    has_greenfield:1;
    guint    has_fec:1;
    guint    has_stbc_streams:1;
    guint    has_ness:1;

    guint16  mcs_index;      /* MCS index */
    guint    bandwidth;      /* Bandwidth = 20 MHz, 40 MHz, etc. */
    guint    short_gi:1;     /* True for short guard interval */
    guint    greenfield:1;   /* True for greenfield, short for mixed */
    guint    fec:1;          /* FEC: 0 = BCC, 1 = LDPC */
    guint    stbc_streams:2; /* Number of STBC streams */
    guint    ness;           /* Number of extension spatial streams */
};

union ieee_802_11_phy_info {
	struct ieee_802_11_fhss info_11_fhss;
	struct ieee_802_11b info_11b;
	struct ieee_802_11a info_11a;
	struct ieee_802_11g info_11g;
	struct ieee_802_11n info_11n;
	struct ieee_802_11ac info_11ac;
	struct ieee_802_11ad info_11ad;
};
struct ieee_802_11b {
	/* Which of this information is present? */
	guint    has_short_preamble : 1;

	gboolean short_preamble; /* Short preamble */
};

/*
* 802.11a.
*/
struct ieee_802_11a {
	/* Which of this information is present? */
	guint    has_channel_type : 1;
	guint    has_turbo_type : 1;

	guint    channel_type : 2;
	guint    turbo_type : 2;
};
struct ieee_802_11_phdr {
	gint     fcs_len;          /* Number of bytes of FCS - -1 means "unknown" */
	gboolean decrypted;        /* TRUE if frame is decrypted even if "protected" bit is set */
	gboolean datapad;          /* TRUE if frame has padding between 802.11 header and payload */
	guint    phy;              /* PHY type */
	union ieee_802_11_phy_info phy_info;

	/* Which of this information is present? */
	guint    has_channel : 1;
	guint    has_frequency : 1;
	guint    has_data_rate : 1;
	guint    has_signal_percent : 1;
	guint    has_noise_percent : 1;
	guint    has_signal_dbm : 1;
	guint    has_noise_dbm : 1;
	guint    has_tsf_timestamp : 1;
	guint    has_aggregate_info : 1;        /* aggregate flags and ID */
	guint    has_zero_length_psdu_type : 1; /* zero-length PSDU type */

	guint16  channel;                     /* Channel number */
	guint32  frequency;                   /* Channel center frequency */
	guint16  data_rate;                   /* Data rate, in .5 Mb/s units */
	guint8   signal_percent;              /* Signal level, as a percentage */
	guint8   noise_percent;               /* Noise level, as a percentage */
	gint8    signal_dbm;                  /* Signal level, in dBm */
	gint8    noise_dbm;                   /* Noise level, in dBm */
	guint64  tsf_timestamp;
	guint32  aggregate_flags;             /* A-MPDU flags */
	guint32  aggregate_id;                /* ID for A-MPDU reassembly */
	guint8   zero_length_psdu_type;       /* type of zero-length PSDU */
};
#define COSINE_MAX_IF_NAME_LEN  128
struct cosine_phdr {
	guint8  encap;      /* COSINE_ENCAP_* as defined above */
	guint8  direction;  /* COSINE_DIR_*, as defined above */
	char    if_name[COSINE_MAX_IF_NAME_LEN];  /* Encap & Logical I/F name */
	guint16 pro;        /* Protocol */
	guint16 off;        /* Offset */
	guint16 pri;        /* Priority */
	guint16 rm;         /* Rate Marking */
	guint16 err;        /* Error Code */
};
struct irda_phdr {
	guint16 pkttype;    /* packet type */
};

/* Packet "pseudo-header" for nettl (HP-UX) capture files. */

struct nettl_phdr {
	guint16 subsys;
	guint32 devid;
	guint32 kind;
	gint32  pid;
	guint16 uid;
};
struct mtp2_phdr {
	guint8  sent;
	guint8  annex_a_used;
	guint16 link_number;
};

/* Packet "pseudo-header" for K12 files. */

typedef union {
	struct {
		guint16 vp;
		guint16 vc;
		guint16 cid;
	} atm;

	guint32 ds0mask;
} k12_input_info_t;

struct k12_phdr {
	guint32           input;
	const gchar      *input_name;
	const gchar      *stack_file;
	guint32           input_type;
	k12_input_info_t  input_info;
	guint8           *extra_info;
	guint32           extra_length;
	void*             stuff;
};
struct lapd_phdr {
	guint16 pkttype;    /* packet type */
	guint8 we_network;
};
struct catapult_dct2000_phdr
{
	union
	{
		struct isdn_phdr isdn;
		struct atm_phdr  atm;
		struct p2p_phdr  p2p;
	} inner_pseudo_header;
	gint64       seek_off;
	struct wtap *wth;
};
/*
* Endace Record Format pseudo header
*/
struct erf_phdr {
	guint64 ts;     /* Time stamp */
	guint8  type;
	guint8  flags;
	guint16 rlen;
	guint16 lctr;
	guint16 wlen;
};
/*
* ERF pseudo header with optional subheader
* (Multichannel or Ethernet)
*/
struct erf_ehdr {
	guint64 ehdr;
};
#define MAX_ERF_EHDR 16
struct erf_mc_phdr {
	struct erf_phdr phdr;
	struct erf_ehdr ehdr_list[MAX_ERF_EHDR];
	union
	{
		struct wtap_erf_eth_hdr eth_hdr;
		guint32 mc_hdr;
		guint32 aal2_hdr;
	} subhdr;
};
struct wtap_erf_eth_hdr {
	guint8 offset;
	guint8 pad;
};
struct sita_phdr {
	guint8  sita_flags;
	guint8  sita_signals;
	guint8  sita_errors1;
	guint8  sita_errors2;
	guint8  sita_proto;
};
struct bthci_phdr {
	gboolean  sent;
	guint32   channel;
};
struct btmon_phdr {
	guint16   adapter_id;
	guint16   opcode;
};

/* pseudo header for WTAP_ENCAP_LAYER1_EVENT */
struct l1event_phdr {
	gboolean uton;
};

/* * I2C pseudo header */
struct i2c_phdr {
	guint8  is_event;
	guint8  bus;
	guint32 flags;
};

/* pseudo header for WTAP_ENCAP_GSM_UM */
struct gsm_um_phdr {
	gboolean uplink;
	guint8   channel;
	/* The following are only populated for downlink */
	guint8   bsic;
	guint16  arfcn;
	guint32  tdma_frame;
	guint8   error;
	guint16  timeshift;
};
struct nstr_phdr {
	gint64 rec_offset;
	gint32 rec_len;
	guint8 nicno_offset;
	guint8 nicno_len;
	guint8 dir_offset;
	guint8 dir_len;
	guint16 eth_offset;
	guint8 pcb_offset;
	guint8 l_pcb_offset;
	guint8 rec_type;
	guint8 vlantag_offset;
	guint8 coreid_offset;
	guint8 srcnodeid_offset;
	guint8 destnodeid_offset;
	guint8 clflags_offset;
	guint8 src_vmname_len_offset;
	guint8 dst_vmname_len_offset;
	guint8 ns_activity_offset;
	guint8 data_offset;
};
struct nokia_phdr {
	struct eth_phdr eth;
	guint8 stuff[4];    /* mysterious stuff */
};

#define LLCP_PHDR_FLAG_SENT 0
struct llcp_phdr {
	guint8 adapter;
	guint8 flags;
};

/* pseudo header for WTAP_ENCAP_LOGCAT */
struct logcat_phdr {
	gint version;
};
struct netmon_phdr {
	guint8* title;          /* Comment title, as a null-terminated UTF-8 string */
	guint32 descLength;     /* Number of bytes in the comment description */
	guint8* description;    /* Comment description, in ASCII RTF */
	guint sub_encap;        /* "Real" encap value for the record that will be used once pseudo header data is display */
	union sub_wtap_pseudo_header {
		struct eth_phdr     eth;
		struct atm_phdr     atm;
		struct ieee_802_11_phdr ieee_802_11;
	} subheader;
};
union wtap_pseudo_header {
	struct eth_phdr     eth;
	struct x25_phdr     x25;
	struct isdn_phdr    isdn;
	struct atm_phdr     atm;
	struct ascend_phdr  ascend;
	struct p2p_phdr     p2p;
	struct ieee_802_11_phdr ieee_802_11;
	struct cosine_phdr  cosine;
	struct irda_phdr    irda;
	struct nettl_phdr   nettl;
	struct mtp2_phdr    mtp2;
	struct k12_phdr     k12;
	struct lapd_phdr    lapd;
	struct catapult_dct2000_phdr dct2000;
	struct erf_mc_phdr  erf;
	struct sita_phdr    sita;
	struct bthci_phdr   bthci;
	struct btmon_phdr   btmon;
	struct l1event_phdr l1event;
	struct i2c_phdr     i2c;
	struct gsm_um_phdr  gsm_um;
	struct nstr_phdr    nstr;
	struct nokia_phdr   nokia;
	struct llcp_phdr    llcp;
	struct logcat_phdr  logcat;
	struct netmon_phdr  netmon;
};

typedef struct {
	guint32   caplen;           /* data length in the file */
	guint32   len;              /* data length on the wire */
	int       pkt_encap;        /* WTAP_ENCAP_ value for this packet */
								/* pcapng variables */
	guint32   interface_id;     /* identifier of the interface. */
								/* options */
	guint64   drop_count;       /* number of packets lost (by the interface and the
								operating system) between this packet and the preceding one. */
	guint32   pack_flags;       /* XXX - 0 for now (any value for "we don't have it"?) */

	union wtap_pseudo_header pseudo_header;
} wtap_packet_header;
typedef struct {
	guint     record_type;      /* the type of record this is - file type-specific value */
} wtap_ft_specific_header;
typedef struct {
	guint     record_type;      /* XXX match ft_specific_record_phdr so that we chain off of packet-pcapng_block for now. */
	int       byte_order;
	/* guint32 sentinel; */
	guint64   timestamp;        /* ns since epoch - XXX dup of ts */
	guint64   thread_id;
	guint32   event_len;        /* length of the event */
	guint32   event_filelen;    /* event data length in the file */
	guint16   event_type;
	guint16   cpu_id;
	/* ... Event ... */
} wtap_syscall_header;

typedef struct Buffer {
	guint8	*data;
	gsize	allocated;
	gsize	start;
	gsize	first_free;
} Buffer;

typedef struct {
	guint     rec_type;         /* what type of record is this? */
	guint32   presence_flags;   /* what stuff do we have? */
	nstime_t  ts;               /* time stamp */
	int       tsprec;           /* WTAP_TSPREC_ value for this record */
	union {
		wtap_packet_header packet_header;
		wtap_ft_specific_header ft_specific_header;
		wtap_syscall_header syscall_header;
	} rec_header;
	/*
	* XXX - this should become a full set of options.
	*/
	gchar     *opt_comment;     /* NULL if not available */
	gboolean  has_comment_changed; /* TRUE if the comment has been changed. Currently only valid while dumping. */

								   /*
								   * We use a Buffer so that we don't have to allocate and free
								   * a buffer for the options for each record.
								   */
	Buffer    options_buf;      /* file-type specific data */
} wtap_rec;

typedef struct wtap_reader *FILE_T;
typedef gboolean(*subtype_read_func)(struct wtap*, int*, char**, gint64*);
typedef gboolean(*subtype_seek_read_func)(struct wtap*, gint64, wtap_rec *,
	Buffer *, int *, char **);
typedef void(*wtap_new_ipv4_callback_t) (const guint addr, const gchar *name);
typedef void(*wtap_new_ipv6_callback_t) (const void *addrp, const gchar *name);
struct wtap {
	FILE_T                      fh;
	FILE_T                      random_fh;              /**< Secondary FILE_T for random access */
	gboolean                    ispipe;                 /**< TRUE if the file is a pipe */
	int                         file_type_subtype;
	guint                       snapshot_length;
	wtap_rec                    rec;
	Buffer                      *rec_data;
	GArray                      *shb_hdrs;
	GArray                      *interface_data;        /**< An array holding the interface data from pcapng IDB:s or equivalent(?)*/
	GArray                      *nrb_hdrs;              /**< holds the Name Res Block's comment/custom_opts, or NULL */

	void                        *priv;          /* this one holds per-file state and is free'd automatically by wtap_close() */
	void                        *wslua_data;    /* this one holds wslua state info and is not free'd */

	subtype_read_func           subtype_read;
	subtype_seek_read_func      subtype_seek_read;
	void(*subtype_sequential_close)(struct wtap*);
	void(*subtype_close)(struct wtap*);
	int                         file_encap;    /* per-file, for those
											   * file formats that have
											   * per-file encapsulation
											   * types rather than per-packet
											   * encapsulation types
											   */
	int                         file_tsprec;   /* per-file timestamp precision
											   * of the fractional part of
											   * the time stamp, for those
											   * file formats that have
											   * per-file timestamp
											   * precision rather than
											   * per-packet timestamp
											   * precision
											   * e.g. WTAP_TSPREC_USEC
											   */
	wtap_new_ipv4_callback_t    add_new_ipv4;
	wtap_new_ipv6_callback_t    add_new_ipv6;
	GPtrArray                   *fast_seek;
};

typedef struct _frame_data {
	GSList      *pfd;          /**< Per frame proto data */
	guint32      num;          /**< Frame number */
	guint32      pkt_len;      /**< Packet length */
	guint32      cap_len;      /**< Amount actually captured */
	guint32      cum_bytes;    /**< Cumulative bytes into the capture */
	gint64       file_off;     /**< File offset */
	guint16      subnum;       /**< subframe number, for protocols that require this */
	gint16       tsprec;       /**< Time stamp precision */
	struct {
		unsigned int passed_dfilter : 1; /**< 1 = display, 0 = no display */
		unsigned int dependent_of_displayed : 1; /**< 1 if a displayed frame depends on this frame */
												 /* Do NOT use packet_char_enc enum here: MSVC compiler does not handle an enum in a bit field properly */
		unsigned int encoding : 1; /**< Character encoding (ASCII, EBCDIC...) */
		unsigned int visited : 1; /**< Has this packet been visited yet? 1=Yes,0=No*/
		unsigned int marked : 1; /**< 1 = marked by user, 0 = normal */
		unsigned int ref_time : 1; /**< 1 = marked as a reference time frame, 0 = normal */
		unsigned int ignored : 1; /**< 1 = ignore this frame, 0 = normal */
		unsigned int has_ts : 1; /**< 1 = has time stamp, 0 = no time stamp */
		unsigned int has_phdr_comment : 1; /** 1 = there's comment for this packet */
		unsigned int has_user_comment : 1; /** 1 = user set (also deleted) comment for this packet */
		unsigned int need_colorize : 1; /**< 1 = need to (re-)calculate packet color */
	} flags;

	const struct _color_filter *color_filter;  /**< Per-packet matching color_filter_t object */

	nstime_t     abs_ts;       /**< Absolute timestamp */
	nstime_t     shift_offset; /**< How much the abs_tm of the frame is shifted */
	guint32      frame_ref_num; /**< Previous reference frame (0 if this is one) */
	guint32      prev_dis_num; /**< Previous displayed frame (0 if first one) */
} frame_data;

struct packet_provider_data {
	wtap        *wth;                  /* Wiretap session */
	const frame_data *ref;
	frame_data  *prev_dis;
	frame_data  *prev_cap;
	frame_data_sequence *frames;       /* Sequence of frames, if we're keeping that information */
	GTree       *frames_user_comments; /* BST with user comments for frames (key = frame_data) */
};

struct _frame_data_sequence {
	guint32      count;           /* Total number of frames */
	void        *ptree_root;      /* Pointer to the root node */
};
typedef struct _frame_data_sequence frame_data_sequence;

struct packet_provider_funcs {
	const nstime_t *(*get_frame_ts)(struct packet_provider_data *prov, guint32 frame_num);
	const char *(*get_interface_name)(struct packet_provider_data *prov, guint32 interface_id);
	const char *(*get_interface_description)(struct packet_provider_data *prov, guint32 interface_id);
	const char *(*get_user_comment)(struct packet_provider_data *prov, const frame_data *fd);
};

struct epan_session {
	struct packet_provider_data *prov;	/* packet provider data for this session */
	struct packet_provider_funcs funcs;	/* functions using that data */
};

typedef struct epan_session epan_t;
typedef enum {
	FILE_CLOSED,                  /* No file open */
	FILE_READ_IN_PROGRESS,        /* Reading a file we've opened */
	FILE_READ_ABORTED,            /* Read aborted by user */
	FILE_READ_DONE                /* Read completed */
} file_state;
/* Passed back to user */
struct epan_dfilter {
	GPtrArray	*insns;
	GPtrArray	*consts;
	guint		num_registers;
	guint		max_registers;
	GList		**registers;
	gboolean	*attempted_load;
	gboolean	*owns_memory;
	int		*interesting_fields;
	int		num_interesting_fields;
	GPtrArray	*deprecated;
};
typedef struct epan_dfilter dfilter_t;
typedef enum {
	RESCAN_NONE = 0,              /* No rescan requested */
	RESCAN_SCAN,                  /* Request rescan without full redissection. */
	RESCAN_REDISSECT              /* Request full redissection. */
} rescan_type;

typedef enum {
	SCS_NARROW_AND_WIDE,
	SCS_NARROW,
	SCS_WIDE
	/* add EBCDIC when it's implemented */
} search_charset_t;

typedef enum {
	SD_FORWARD,
	SD_BACKWARD
} search_direction;
typedef struct {
	const gchar **col_expr;     /**< Filter expression */
	gchar      **col_expr_val;  /**< Value for filter expression */
} col_expr_t;

/** Individual column info */
typedef struct {
	gint                col_fmt;              /**< Format of column */
	gboolean           *fmt_matx;             /**< Specifies which formats apply to a column */
	gchar              *col_title;            /**< Column titles */
	gchar              *col_custom_fields;    /**< Custom column fields */
	gint                col_custom_occurrence;/**< Custom column field occurrence */
	GSList             *col_custom_fields_ids;/**< Custom column fields id */
	struct epan_dfilter *col_custom_dfilter;  /**< Compiled custom column field */
	const gchar        *col_data;             /**< Column data */
	gchar              *col_buf;              /**< Buffer into which to copy data for column */
	int                 col_fence;            /**< Stuff in column buffer before this index is immutable */
	gboolean            writable;             /**< writable or not */
} col_item_t;
struct epan_column_info {
	const struct epan_session *epan;
	gint                num_cols;             /**< Number of columns */
	col_item_t         *columns;              /**< All column data */
	gint               *col_first;            /**< First column number with a given format */
	gint               *col_last;             /**< Last column number with a given format */
	col_expr_t          col_expr;             /**< Column expressions and values */
	gboolean            writable;             /**< writable or not @todo Are we still writing to the columns? */
	GRegex             *prime_regex;          /**< Used to prime custom columns */
};
typedef struct epan_column_info column_info;
struct tvbuff {
	/* Doubly linked list pointers */
	tvbuff_t                *next;

	/* Record-keeping */
	const struct tvb_ops   *ops;
	gboolean		initialized;
	guint			flags;
	struct tvbuff		*ds_tvb;  /**< data source top-level tvbuff */

								  /** Pointer to the data for this tvbuff.
								  * It might be null, which either means that 1) it's a
								  * zero-length tvbuff or 2) the tvbuff was lazily
								  * constructed, so that we don't allocate a buffer of
								  * backing data and fill it in unless we need that
								  * data, e.g. when tvb_get_ptr() is called.
								  */
	const guint8		*real_data;

	/** Amount of data that's available from the capture
	* file.  This is the length of virtual buffer (and/or
	* real_data).  It may be less than the reported
	* length if this is from a packet that was cut short
	* by the capture process.
	*
	* This must never be > reported_length or contained_length. */
	guint			length;

	/** Amount of data that was reported as being in
	* the packet or other data that this represents.
	* As indicated above, it may be greater than the
	* amount of data that's available. */
	guint			reported_length;

	/** If this was extracted from a parent tvbuff,
	* this is the amount of extracted data that
	* was reported as being in the parent tvbuff;
	* if this represents a blob of data in that
	* tvbuff that has a length specified by data
	* in that tvbuff, it might be greater than
	* the amount of data that was actually there
	* to extract, so it could be greater than
	* reported_length.
	*
	* If this wasn't extracted from a parent tvbuff,
	* this is the same as reported_length.
	*
	* This must never be > reported_length. */
	guint			contained_length;

	/* Offset from beginning of first "real" tvbuff. */
	gint			raw_offset;
};
typedef struct tvbuff tvbuff_t;
/** One of these exists for the entire protocol tree. Each proto_node
* in the protocol tree points to the same copy. */
typedef struct {
	GHashTable  *interesting_hfids;
	gboolean     visible;
	gboolean     fake_protocols;
	gint         count;
	struct _packet_info *pinfo;
} tree_data_t;
typedef struct _header_field_info header_field_info;
#define HFILL -1, 0, HF_REF_TYPE_NONE, -1, NULL

typedef enum {
	HF_REF_TYPE_NONE,       /**< Field is not referenced */
	HF_REF_TYPE_INDIRECT,   /**< Field is indirectly referenced (only applicable for FT_PROTOCOL) via. its child */
	HF_REF_TYPE_DIRECT      /**< Field is directly referenced */
} hf_ref_type;
/** information describing a header field */
enum ftenum {
	FT_NONE,	/* used for text labels with no value */
	FT_PROTOCOL,
	FT_BOOLEAN,	/* TRUE and FALSE come from <glib.h> */
	FT_CHAR,	/* 1-octet character as 0-255 */
	FT_UINT8,
	FT_UINT16,
	FT_UINT24,	/* really a UINT32, but displayed as 6 hex-digits if FD_HEX*/
	FT_UINT32,
	FT_UINT40,	/* really a UINT64, but displayed as 10 hex-digits if FD_HEX*/
	FT_UINT48,	/* really a UINT64, but displayed as 12 hex-digits if FD_HEX*/
	FT_UINT56,	/* really a UINT64, but displayed as 14 hex-digits if FD_HEX*/
	FT_UINT64,
	FT_INT8,
	FT_INT16,
	FT_INT24,	/* same as for UINT24 */
	FT_INT32,
	FT_INT40, /* same as for UINT40 */
	FT_INT48, /* same as for UINT48 */
	FT_INT56, /* same as for UINT56 */
	FT_INT64,
	FT_IEEE_11073_SFLOAT,
	FT_IEEE_11073_FLOAT,
	FT_FLOAT,
	FT_DOUBLE,
	FT_ABSOLUTE_TIME,
	FT_RELATIVE_TIME,
	FT_STRING,
	FT_STRINGZ,	/* for use with proto_tree_add_item() */
	FT_UINT_STRING,	/* for use with proto_tree_add_item() */
	FT_ETHER,
	FT_BYTES,
	FT_UINT_BYTES,
	FT_IPv4,
	FT_IPv6,
	FT_IPXNET,
	FT_FRAMENUM,	/* a UINT32, but if selected lets you go to frame with that number */
	FT_PCRE,	/* a compiled Perl-Compatible Regular Expression object */
	FT_GUID,	/* GUID, UUID */
	FT_OID,		/* OBJECT IDENTIFIER */
	FT_EUI64,
	FT_AX25,
	FT_VINES,
	FT_REL_OID,	/* RELATIVE-OID */
	FT_SYSTEM_ID,
	FT_STRINGZPAD,	/* for use with proto_tree_add_item() */
	FT_FCWWN,
	FT_NUM_TYPES /* last item number plus one */
};
struct _header_field_info {
	/* ---------- set by dissector --------- */
	const char		*name;              /**< [FIELDNAME] full name of this field */
	const char		*abbrev;            /**< [FIELDABBREV] abbreviated name of this field */
	enum ftenum		 type;              /**< [FIELDTYPE] field type, one of FT_ (from ftypes.h) */
	int			 display;           /**< [FIELDDISPLAY] one of BASE_, or field bit-width if FT_BOOLEAN and non-zero bitmask */
	const void		*strings;           /**< [FIELDCONVERT] value_string, val64_string, range_string or true_false_string,
										typically converted by VALS(), RVALS() or TFS().
										If this is an FT_PROTOCOL or BASE_PROTOCOL_INFO then it points to the
										associated protocol_t structure */
	guint64			 bitmask;           /**< [BITMASK] bitmask of interesting bits */
	const char		*blurb;             /**< [FIELDDESCR] Brief description of field */

										/* ------- set by proto routines (prefilled by HFILL macro, see below) ------ */
	int			 id;                /**< Field ID */
	int			 parent;            /**< parent protocol tree */
	hf_ref_type		 ref_type;          /**< is this field referenced by a filter */
	int			 same_name_prev_id; /**< ID of previous hfinfo with same abbrev */
	header_field_info	*same_name_next;    /**< Link to next hfinfo with same abbrev */
};
#define ITEM_LABEL_LENGTH	240

typedef struct _item_label_t {
	char representation[ITEM_LABEL_LENGTH];
} item_label_t;
typedef enum ftenum ftenum_t;
typedef void(*FvalueNewFunc)(fvalue_t*);
typedef void(*FvalueFreeFunc)(fvalue_t*);

typedef gboolean(*FvalueFromUnparsed)(fvalue_t*, const char*, gboolean, gchar **);
typedef gboolean(*FvalueFromString)(fvalue_t*, const char*, gchar **);
enum ftrepr {
	FTREPR_DISPLAY,
	FTREPR_DFILTER
};

typedef enum ftrepr ftrepr_t;
typedef void(*FvalueToStringRepr)(fvalue_t*, ftrepr_t, int field_display, char*volatile, unsigned int);
typedef int(*FvalueStringReprLen)(fvalue_t*, ftrepr_t, int field_display);

typedef void(*FvalueSetByteArrayFunc)(fvalue_t*, GByteArray *);
typedef void(*FvalueSetBytesFunc)(fvalue_t*, const guint8 *);
typedef struct _e_guid_t {
	guint32 data1;
	guint16 data2;
	guint16 data3;
	guint8  data4[8];
} e_guid_t;
typedef void(*FvalueSetGuidFunc)(fvalue_t*, const e_guid_t *);
typedef void(*FvalueSetTimeFunc)(fvalue_t*, const nstime_t *);
typedef void(*FvalueSetStringFunc)(fvalue_t*, const gchar *value);
typedef void(*FvalueSetProtocolFunc)(fvalue_t*, tvbuff_t *value, const gchar *name);
typedef void(*FvalueSetUnsignedIntegerFunc)(fvalue_t*, guint32);
typedef void(*FvalueSetSignedIntegerFunc)(fvalue_t*, gint32);
typedef void(*FvalueSetUnsignedInteger64Func)(fvalue_t*, guint64);
typedef void(*FvalueSetSignedInteger64Func)(fvalue_t*, gint64);
typedef void(*FvalueSetFloatingFunc)(fvalue_t*, gdouble);
typedef gpointer(*FvalueGetFunc)(fvalue_t*);
typedef guint32(*FvalueGetUnsignedIntegerFunc)(fvalue_t*);
typedef gint32(*FvalueGetSignedIntegerFunc)(fvalue_t*);
typedef guint64(*FvalueGetUnsignedInteger64Func)(fvalue_t*);
typedef gint64(*FvalueGetSignedInteger64Func)(fvalue_t*);
typedef double(*FvalueGetFloatingFunc)(fvalue_t*);

typedef gboolean(*FvalueCmp)(const fvalue_t*, const fvalue_t*);

typedef guint(*FvalueLen)(fvalue_t*);
typedef void(*FvalueSlice)(fvalue_t*, GByteArray *, guint offset, guint length);
struct _ftype_t {
	ftenum_t		ftype;
	const char		*name;
	const char		*pretty_name;
	int			wire_size;
	FvalueNewFunc		new_value;
	FvalueFreeFunc		free_value;
	FvalueFromUnparsed	val_from_unparsed;
	FvalueFromString	val_from_string;
	FvalueToStringRepr	val_to_string_repr;
	FvalueStringReprLen	len_string_repr;

	union {
		FvalueSetByteArrayFunc	set_value_byte_array;
		FvalueSetBytesFunc	set_value_bytes;
		FvalueSetGuidFunc	set_value_guid;
		FvalueSetTimeFunc	set_value_time;
		FvalueSetStringFunc	set_value_string;
		FvalueSetProtocolFunc	set_value_protocol;
		FvalueSetUnsignedIntegerFunc	set_value_uinteger;
		FvalueSetSignedIntegerFunc	set_value_sinteger;
		FvalueSetUnsignedInteger64Func	set_value_uinteger64;
		FvalueSetSignedInteger64Func	set_value_sinteger64;
		FvalueSetFloatingFunc	set_value_floating;
	} set_value;

	union {
		FvalueGetFunc		get_value_ptr;
		FvalueGetUnsignedIntegerFunc	get_value_uinteger;
		FvalueGetSignedIntegerFunc	get_value_sinteger;
		FvalueGetUnsignedInteger64Func	get_value_uinteger64;
		FvalueGetSignedInteger64Func	get_value_sinteger64;
		FvalueGetFloatingFunc	get_value_floating;
	} get_value;

	FvalueCmp		cmp_eq;
	FvalueCmp		cmp_ne;
	FvalueCmp		cmp_gt;
	FvalueCmp		cmp_ge;
	FvalueCmp		cmp_lt;
	FvalueCmp		cmp_le;
	FvalueCmp		cmp_bitwise_and;
	FvalueCmp		cmp_contains;
	FvalueCmp		cmp_matches;

	FvalueLen		len;
	FvalueSlice		slice;
};
typedef struct _ftype_t ftype_t;
typedef struct {
	guint32	addr;	/* stored in host order */
	guint32	nmask;	/* stored in host order */
} ipv4_addr_and_mask;
typedef struct e_in6_addr {
	guint8 bytes[16];           /* 128 bit IPv6 address */
} ws_in6_addr;
typedef struct {
	ws_in6_addr addr;
	guint32 prefix;
} ipv6_addr_and_prefix;
typedef struct _protocol_value_t
{
	tvbuff_t	*tvb;
	gchar		*proto_string;
} protocol_value_t;
typedef struct _fvalue_t {
	ftype_t	*ftype;
	union {
		/* Put a few basic types in here */
		guint32			uinteger;
		gint32			sinteger;
		guint64			integer64;
		guint64			uinteger64;
		gint64			sinteger64;
		gdouble			floating;
		gchar			*string;
		guchar			*ustring;
		GByteArray		*bytes;
		ipv4_addr_and_mask	ipv4;
		ipv6_addr_and_prefix	ipv6;
		e_guid_t		guid;
		nstime_t		time;
		protocol_value_t 	protocol;
		GRegex			*re;
		guint16			sfloat_ieee_11073;
		guint32			float_ieee_11073;
	} value;

	/* The following is provided for private use
	* by the fvalue. */
	gboolean	fvalue_gboolean1;

} fvalue_t;
typedef struct field_info {
	header_field_info	*hfinfo;          /**< pointer to registered field information */
	gint			 start;           /**< current start of data in field_info.ds_tvb */
	gint			 length;          /**< current data length of item in field_info.ds_tvb */
	gint			 appendix_start;  /**< start of appendix data */
	gint			 appendix_length; /**< length of appendix data */
	gint			 tree_type;       /**< one of ETT_ or -1 */
	guint32			 flags;           /**< bitfield like FI_GENERATED, ... */
	item_label_t		*rep;             /**< string for GUI tree */
	tvbuff_t		*ds_tvb;          /**< data source tvbuff */
	fvalue_t		 value;
} field_info;
/** Each proto_tree, proto_item is one of these. */
typedef struct _proto_node {
	struct _proto_node *first_child;
	struct _proto_node *last_child;
	struct _proto_node *next;
	struct _proto_node *parent;
	field_info  *finfo;
	tree_data_t *tree_data;
} proto_node;

/** A protocol tree element. */
typedef proto_node proto_tree;
typedef struct _address {
	int           type;         /* type of address */
	int           len;          /* length of address, in bytes */
	const void   *data;         /* pointer to address data */

								/* private */
	void         *priv;
} address;
typedef enum {
	PT_NONE,            /* no port number */
	PT_SCTP,            /* SCTP */
	PT_TCP,             /* TCP */
	PT_UDP,             /* UDP */
	PT_DCCP,            /* DCCP */
	PT_IPX,             /* IPX sockets */
	PT_DDP,             /* DDP AppleTalk connection */
	PT_IDP,             /* XNS IDP sockets */
	PT_USB,             /* USB endpoint 0xffff means the host */
	PT_I2C,
	PT_IBQP,            /* Infiniband QP number */
	PT_BLUETOOTH
} port_type;
struct _wmem_list_frame_t {
	struct _wmem_list_frame_t *next, *prev;
	void *data;
};
typedef struct _wmem_list_frame_t wmem_list_frame_t;
typedef enum _wmem_allocator_type_t {
	WMEM_ALLOCATOR_SIMPLE, /**< A trivial allocator that mallocs requested
						   memory and tracks allocations via a hash table. As simple as
						   possible, intended more as a demo than for practical usage. Also
						   has the benefit of being friendly to tools like valgrind. */
	WMEM_ALLOCATOR_BLOCK, /**< A block allocator that grabs large chunks of
						  memory at a time (8 MB currently) and serves allocations out of
						  those chunks. Designed for efficiency, especially in the
						  free_all operation. */
	WMEM_ALLOCATOR_STRICT, /**< An allocator that does its best to find invalid
						   memory usage via things like canaries and scrubbing freed
						   memory. Valgrind is the better choice on platforms that support
						   it. */
	WMEM_ALLOCATOR_BLOCK_FAST /**< A block allocator like WMEM_ALLOCATOR_BLOCK
							  but even faster by tracking absolutely minimal metadata and
							  making 'free' a no-op. Useful only for very short-lived scopes
							  where there's no reason to free individual allocations because
							  the next free_all is always just around the corner. */
} wmem_allocator_type_t;
struct _wmem_allocator_t {
	/* Consumer functions */
	void *(*walloc)(void *private_data, const size_t size);
	void(*wfree)(void *private_data, void *ptr);
	void *(*wrealloc)(void *private_data, void *ptr, const size_t size);

	/* Producer/Manager functions */
	void(*free_all)(void *private_data);
	void(*gc)(void *private_data);
	void(*cleanup)(void *private_data);

	/* Callback List */
	struct _wmem_user_cb_container_t *callbacks;

	/* Implementation details */
	void                        *private_data;
	enum _wmem_allocator_type_t  type;
	gboolean                     in_scope;
};
typedef struct _wmem_allocator_t wmem_allocator_t;

struct _wmem_list_t {
	guint count;
	wmem_list_frame_t  *head, *tail;
	wmem_allocator_t   *allocator;
};
typedef struct _wmem_list_t       wmem_list_t;
typedef struct _packet_info {
	const char *current_proto;        /**< name of protocol currently being dissected */
	struct epan_column_info *cinfo;   /**< Column formatting information */
	guint32 presence_flags;           /**< Presence flags for some items */
	guint32 num;                      /**< Frame number */
	nstime_t abs_ts;                  /**< Packet absolute time stamp */
	nstime_t rel_ts;                  /**< Relative timestamp (yes, it can be negative) */
	frame_data *fd;
	union wtap_pseudo_header *pseudo_header;
	wtap_rec *rec;                    /**< Record metadata */
	GSList *data_src;                 /**< Frame data sources */
	address dl_src;                   /**< link-layer source address */
	address dl_dst;                   /**< link-layer destination address */
	address net_src;                  /**< network-layer source address */
	address net_dst;                  /**< network-layer destination address */
	address src;                      /**< source address (net if present, DL otherwise )*/
	address dst;                      /**< destination address (net if present, DL otherwise )*/
	guint32 vlan_id;                  /**< First encountered VLAN Id if present otherwise 0 */
	const char *noreassembly_reason;  /**< reason why reassembly wasn't done, if any */
	gboolean fragmented;              /**< TRUE if the protocol is only a fragment */
	struct {
		guint32 in_error_pkt : 1;         /**< TRUE if we're inside an {ICMP,CLNP,...} error packet */
		guint32 in_gre_pkt : 1;           /**< TRUE if we're encapsulated inside a GRE packet */
	} flags;
	port_type ptype;                  /**< type of the following two port numbers */
	guint32 srcport;                  /**< source port */
	guint32 destport;                 /**< destination port */
	guint32 match_uint;               /**< matched uint for calling subdissector from table */
	const char *match_string;         /**< matched string for calling subdissector from table */
	gboolean use_endpoint;            /**< TRUE if endpoint member should be used for conversations */
	struct endpoint* conv_endpoint;   /**< Data that can be used for conversations */
	guint16 can_desegment;            /**< >0 if this segment could be desegmented.
									  A dissector that can offer this API (e.g.
									  TCP) sets can_desegment=2, then
									  can_desegment is decremented by 1 each time
									  we pass to the next subdissector. Thus only
									  the dissector immediately above the
									  protocol which sets the flag can use it*/
	guint16 saved_can_desegment;      /**< Value of can_desegment before current
									  dissector was called.  Supplied so that
									  dissectors for proxy protocols such as
									  SOCKS can restore it, allowing the
									  dissectors that they call to use the
									  TCP dissector's desegmentation (SOCKS
									  just retransmits TCP segments once it's
									  finished setting things up, so the TCP
									  desegmentor can desegment its payload). */
	int desegment_offset;             /**< offset to stuff needing desegmentation */
#define DESEGMENT_ONE_MORE_SEGMENT 0x0fffffff
#define DESEGMENT_UNTIL_FIN        0x0ffffffe
	guint32 desegment_len;            /**< requested desegmentation additional length
									  or
									  DESEGMENT_ONE_MORE_SEGMENT:
									  Desegment one more full segment
									  (warning! only partially implemented)
									  DESEGMENT_UNTIL_FIN:
									  Desgment all data for this tcp session
									  until the FIN segment.
									  */
	guint16 want_pdu_tracking;    /**< >0 if the subdissector has specified
								  a value in 'bytes_until_next_pdu'.
								  When a dissector detects that the next PDU
								  will start beyond the start of the next
								  segment, it can set this value to 2
								  and 'bytes_until_next_pdu' to the number of
								  bytes beyond the next segment where the
								  next PDU starts.

								  If the protocol dissector below this
								  one is capable of PDU tracking it can
								  use this hint to detect PDUs that starts
								  unaligned to the segment boundaries.
								  The TCP dissector is using this hint from
								  (some) protocols to detect when a new PDU
								  starts in the middle of a tcp segment.

								  There is intelligence in the glue between
								  dissector layers to make sure that this
								  request is only passed down to the protocol
								  immediately below the current one and not
								  any further.
								  */
	guint32 bytes_until_next_pdu;

	int     p2p_dir;              /**< Packet was captured as an
								  outbound (P2P_DIR_SENT)
								  inbound (P2P_DIR_RECV)
								  unknown (P2P_DIR_UNKNOWN) */

	GHashTable *private_table;    /**< a hash table passed from one dissector to another */

	wmem_list_t *layers;      /**< layers of each protocol */
	guint8 curr_layer_num;       /**< The current "depth" or layer number in the current frame */
	guint16 link_number;

	guint16 clnp_srcref;          /**< clnp/cotp source reference (can't use srcport, this would confuse tpkt) */
	guint16 clnp_dstref;          /**< clnp/cotp destination reference (can't use dstport, this would confuse tpkt) */

	int link_dir;                 /**< 3GPP messages are sometime different UP link(UL) or Downlink(DL) */

	GSList* proto_data;          /**< Per packet proto data */

	GSList* dependent_frames;     /**< A list of frames which this one depends on */

	GSList* frame_end_routines;

	wmem_allocator_t *pool;      /**< Memory pool scoped to the pinfo struct */
	struct epan_session *epan;
	const gchar *heur_list_name;    /**< name of heur list if this packet is being heuristically dissected */
} packet_info;
struct epan_dissect {
	struct epan_session *session;
	tvbuff_t	*tvb;
	proto_tree	*tree;
	packet_info	pi;
};
typedef struct epan_dissect epan_dissect_t;
typedef struct _capture_file {
	epan_t      *epan;
	file_state   state;                /* Current state of capture file */
	gchar       *filename;             /* Name of capture file */
	gchar       *source;               /* Temp file source, e.g. "Pipe from elsewhere" */
	gboolean     is_tempfile;          /* Is capture file a temporary file? */
	gboolean     unsaved_changes;      /* Does the capture file have changes that have not been saved? */
	gboolean     stop_flag;            /* Stop current processing (loading, searching, etc.) */

	gint64       f_datalen;            /* Size of capture file data (uncompressed) */
	guint16      cd_t;                 /* File type of capture file */
	unsigned int open_type;            /* open_routine index+1 used, if selected, or WTAP_TYPE_AUTO */
	gboolean     iscompressed;         /* TRUE if the file is compressed */
	int          lnk_t;                /* File link-layer type; could be WTAP_ENCAP_PER_PACKET */
	GArray      *linktypes;            /* Array of packet link-layer types */
	guint32      count;                /* Total number of frames */
	guint64      packet_comment_count; /* Number of comments in frames (could be >1 per frame... */
	guint32      displayed_count;      /* Number of displayed frames */
	guint32      marked_count;         /* Number of marked frames */
	guint32      ignored_count;        /* Number of ignored frames */
	guint32      ref_time_count;       /* Number of time referenced frames */
	gboolean     drops_known;          /* TRUE if we know how many packets were dropped */
	guint32      drops;                /* Dropped packets */
	nstime_t     elapsed_time;         /* Elapsed time */
	int          snap;                 /* Maximum captured packet length; 0 if unknown */
	dfilter_t   *rfcode;               /* Compiled read filter program */
	dfilter_t   *dfcode;               /* Compiled display filter program */
	gchar       *dfilter;              /* Display filter string */
	gboolean     redissecting;         /* TRUE if currently redissecting (cf_redissect_packets) */
	gboolean     read_lock;            /* TRUE if currently processing a file (cf_read) */
	rescan_type  redissection_queued;  /* Queued redissection type. */
									   /* search */
	gchar       *sfilter;              /* Filter, hex value, or string being searched */
	gboolean     hex;                  /* TRUE if "Hex value" search was last selected */
	gboolean     string;               /* TRUE if "String" search was last selected */
	gboolean     summary_data;         /* TRUE if "String" search in "Packet list" (Info column) was last selected */
	gboolean     decode_data;          /* TRUE if "String" search in "Packet details" was last selected */
	gboolean     packet_data;          /* TRUE if "String" search in "Packet data" was last selected */
	guint32      search_pos;           /* Byte position of last byte found in a hex search */
	guint32      search_len;           /* Length of bytes matching the search */
	gboolean     case_type;            /* TRUE if case-insensitive text search */
	GRegex      *regex;                /* Set if regular expression search */
	search_charset_t scs_type;         /* Character set for text search */
	search_direction dir;              /* Direction in which to do searches */
	gboolean     search_in_progress;   /* TRUE if user just clicked OK in the Find dialog or hit <control>N/B */
									   /* packet data */
	wtap_rec     rec;                  /* Record header */
	Buffer       buf;                  /* Record data */
									   /* packet provider */
	struct packet_provider_data provider;
	/* frames */
	guint32      first_displayed;      /* Frame number of first frame displayed */
	guint32      last_displayed;       /* Frame number of last frame displayed */
	column_info  cinfo;                /* Column formatting information */
	frame_data  *current_frame;        /* Frame data for current frame */
	gint         current_row;          /* Row number for current frame */
	epan_dissect_t *edt;               /* Protocol dissection for currently selected packet */
	field_info  *finfo_selected;       /* Field info for currently selected field */
	gpointer     window;               /* Top-level window associated with file */
	gulong       computed_elapsed;     /* Elapsed time to load the file (in msec). */

	guint32      cum_bytes;
} capture_file;
struct wtap* wtap_open_offline(const char *filename, unsigned int type, int *err, gchar **err_info, gboolean do_random);
typedef void(*cf_callback_t) (gint event, gpointer data, gpointer user_data);

typedef struct {
	cf_callback_t cb_fct;
	gpointer      user_data;
} cf_callback_data_t;
static GList *cf_callbacks = NULL;

static void cf_callback_invoke(int event, gpointer data)
{
	cf_callback_data_t *cb;
	GList              *cb_item = cf_callbacks;

	/* there should be at least one interested */
	g_assert(cb_item != NULL);

	while (cb_item != NULL) {
		cb = (cf_callback_data_t *)cb_item->data;
		cb->cb_fct(event, data, cb->user_data);
		cb_item = g_list_next(cb_item);
	}
}
typedef enum {
	cf_cb_file_opened,
	cf_cb_file_closing,
	cf_cb_file_closed,
	cf_cb_file_read_started,
	cf_cb_file_read_finished,
	cf_cb_file_reload_started,
	cf_cb_file_reload_finished,
	cf_cb_file_rescan_started,
	cf_cb_file_rescan_finished,
	cf_cb_file_retap_started,
	cf_cb_file_retap_finished,
	cf_cb_file_merge_started, /* Qt only */
	cf_cb_file_merge_finished, /* Qt only */
	cf_cb_file_fast_save_finished,
	cf_cb_file_save_started,
	cf_cb_file_save_finished,
	cf_cb_file_save_failed,
	cf_cb_file_save_stopped
} cf_cbs;
typedef struct {
	guint16 red;
	guint16 green;
	guint16 blue;
} color_t;
typedef struct _color_filter {
	gchar     *filter_name;         /* name of the filter */
	gchar     *filter_text;         /* text of the filter expression */
	color_t    bg_color;            /* background color for packets that match */
	color_t    fg_color;            /* foreground color for packets that match */
	gboolean   disabled;            /* set if the filter is disabled */

									/* only used inside of color_filters.c */
	struct epan_dfilter *c_colorfilter;  /* compiled filter expression */

										 /* only used outside of color_filters.c (beside init) */
} color_filter_t;
typedef enum {
	EMPTY,
	FVALUE,
	HFINFO,
	INSN_NUMBER,
	REGISTER,
	INTEGER,
	DRANGE,
	FUNCTION_DEF
} dfvm_value_type_t;
typedef struct _drange {
	GSList* range_list;
	gboolean has_total_length;
	gint total_length;
	gint min_start_offset;
	gint max_start_offset;
} drange_t;
typedef gboolean(*DFFuncType)(GList *arg1list, GList *arg2list, GList **retval);
typedef enum {
	STTYPE_UNINITIALIZED,
	STTYPE_TEST,
	STTYPE_UNPARSED,
	STTYPE_STRING,
	STTYPE_CHARCONST,
	STTYPE_FIELD,
	STTYPE_FVALUE,
	STTYPE_INTEGER,
	STTYPE_RANGE,
	STTYPE_FUNCTION,
	STTYPE_SET,
	STTYPE_NUM_TYPES
} sttype_id_t;
typedef gpointer(*STTypeNewFunc)(gpointer);
typedef gpointer(*STTypeDupFunc)(gconstpointer);
typedef void(*STTypeFreeFunc)(gpointer);
typedef struct {
	sttype_id_t		id;
	const char		*name;
	STTypeNewFunc		func_new;
	STTypeFreeFunc		func_free;
	STTypeDupFunc		func_dup;
} sttype_t;
typedef struct {
	guint32		magic;
	sttype_t	*type;

	/* This could be made an enum, but I haven't
	* set aside to time to do so. */
	gpointer	data;
	gint32		value;
	gboolean	inside_brackets;
	const char	*deprecated_token;
} stnode_t;
typedef struct {
	/* Syntax Tree stuff */
	stnode_t	*st_root;
	gboolean	syntax_error;
	gchar		*error_message;
	GPtrArray	*insns;
	GPtrArray	*consts;
	GHashTable	*loaded_fields;
	GHashTable	*interesting_fields;
	int		next_insn_id;
	int		next_const_id;
	int		next_register;
	int		first_constant; /* first register used as a constant */
} dfwork_t;
typedef void(*DFSemCheckType)(dfwork_t *dfw, int param_num, stnode_t *st_node);

typedef struct {
	const char      *name;
	DFFuncType      function;
	ftenum_t        retval_ftype;
	guint           min_nargs;
	guint           max_nargs;
	DFSemCheckType  semcheck_param_function;
} df_func_def_t;
typedef struct {
	dfvm_value_type_t	type;

	union {
		fvalue_t		*fvalue;
		guint32			numeric;
		drange_t		*drange;
		header_field_info	*hfinfo;
		df_func_def_t   *funcdef;
	} value;

} dfvm_value_t;


typedef enum {

	IF_TRUE_GOTO,
	IF_FALSE_GOTO,
	CHECK_EXISTS,
	NOT,
	RETURN,
	READ_TREE,
	PUT_FVALUE,
	ANY_EQ,
	ANY_NE,
	ANY_GT,
	ANY_GE,
	ANY_LT,
	ANY_LE,
	ANY_BITWISE_AND,
	ANY_CONTAINS,
	ANY_MATCHES,
	MK_RANGE,
	CALL_FUNCTION,
	ANY_IN_RANGE

} dfvm_opcode_t;
typedef struct {
	int		id;
	dfvm_opcode_t	op;
	dfvm_value_t	*arg1;
	dfvm_value_t	*arg2;
	dfvm_value_t	*arg3;
	dfvm_value_t	*arg4;
} dfvm_insn_t;
#define FVALUE_CLEANUP(fv)					\
	{							\
		register FvalueFreeFunc	free_value;		\
		free_value = (fv)->ftype->free_value;	\
		if (free_value) {				\
			free_value((fv));			\
		}						\
	}
#define FVALUE_FREE(fv)	{							\
		FVALUE_CLEANUP(fv)				\
		g_slice_free(fvalue_t, fv);			\
	}
void
drange_node_free_list(GSList* list)
{
	g_slist_free_full(list, g_free);
}
void
drange_free(drange_t * dr)
{
	drange_node_free_list(dr->range_list);
	g_free(dr);
}
static void dfvm_value_free(dfvm_value_t *v)
{
	switch (v->type) {
	case FVALUE:
		FVALUE_FREE(v->value.fvalue);
		break;
	case DRANGE:
		drange_free(v->value.drange);
		break;
	default:
		/* nothing */
		;
	}
	g_free(v);
}
void dfvm_insn_free(dfvm_insn_t *insn)
{
	if (insn->arg1) {
		dfvm_value_free(insn->arg1);
	}
	if (insn->arg2) {
		dfvm_value_free(insn->arg2);
	}
	if (insn->arg3) {
		dfvm_value_free(insn->arg3);
	}
	if (insn->arg4) {
		dfvm_value_free(insn->arg4);
	}
	g_free(insn);
}
static void
free_insns(GPtrArray *insns)
{
	unsigned int	i;
	dfvm_insn_t	*insn;

	for (i = 0; i < insns->len; i++) {
		insn = (dfvm_insn_t	*)g_ptr_array_index(insns, i);
		dfvm_insn_free(insn);
	}
	g_ptr_array_free(insns, TRUE);
}
void dfilter_free(dfilter_t *df)
{
	guint i;

	if (!df)
		return;

	if (df->insns) {
		free_insns(df->insns);
	}
	if (df->consts) {
		free_insns(df->consts);
	}

	g_free(df->interesting_fields);

	/* Clear registers with constant values (as set by dfvm_init_const).
	* Other registers were cleared on RETURN by free_register_overhead. */
	for (i = df->num_registers; i < df->max_registers; i++) {
		g_list_free(df->registers[i]);
	}

	if (df->deprecated) {
		for (i = 0; i < df->deprecated->len; ++i) {
			gchar *depr = (gchar *)g_ptr_array_index(df->deprecated, i);
			g_free(depr);
		}
		g_ptr_array_free(df->deprecated, TRUE);
	}

	g_free(df->registers);
	g_free(df->attempted_load);
	g_free(df->owns_memory);
	g_free(df);
}
void
color_filter_delete(color_filter_t *colorf)
{
	g_free(colorf->filter_name);
	g_free(colorf->filter_text);
	dfilter_free(colorf->c_colorfilter);
	g_free(colorf);
}
static void
color_filter_delete_cb(gpointer filter_arg)
{
	color_filter_t *colorf = (color_filter_t *)filter_arg;

	color_filter_delete(colorf);
}
void
color_filter_list_delete(GSList **cfl)
{
	g_slist_free_full(*cfl, color_filter_delete_cb);
	*cfl = NULL;
}
static GSList *color_filter_deleted_list = NULL;
void color_filters_cleanup(void)
{
	/* delete the previously deleted filters */
	color_filter_list_delete(&color_filter_deleted_list);
}
struct wtap_reader_buf {
	guint8 *buf;  /* buffer */
	guint8 *next; /* next byte to deliver from buffer */
	guint avail;  /* number of bytes available to deliver at next */
};
typedef enum {
	UNKNOWN,       /* unknown - look for a gzip header */
	UNCOMPRESSED,  /* uncompressed - copy input directly */
#ifdef HAVE_ZLIB
	ZLIB,          /* decompress a zlib stream */
	GZIP_AFTER_HEADER
#endif
} compression_t;
struct wtap_reader {
	int fd;                     /* file descriptor */
	gint64 raw_pos;             /* current position in file (just to not call lseek()) */
	gint64 pos;                 /* current position in uncompressed data */
	guint size;                 /* buffer size */

	struct wtap_reader_buf in;  /* input buffer, containing compressed data */
	struct wtap_reader_buf out; /* output buffer, containing uncompressed data */

	gboolean eof;               /* TRUE if end of input file reached */
	gint64 start;               /* where the gzip data started, for rewinding */
	gint64 raw;                 /* where the raw data started, for seeking */
	compression_t compression;  /* type of compression, if any */
	gboolean is_compressed;     /* FALSE if completely uncompressed, TRUE otherwise */

								/* seek request */
	gint64 skip;                /* amount to skip (already rewound if backwards) */
	gboolean seek_pending;      /* TRUE if seek request pending */

								/* error information */
	int err;                    /* error code */
	const char *err_info;       /* additional error information string for some errors */

#ifdef HAVE_ZLIB
								/* zlib inflate stream */
	z_stream strm;              /* stream structure in-place (not a pointer) */
	gboolean dont_check_crc;    /* TRUE if we aren't supposed to check the CRC */
#endif
								/* fast seeking */
	GPtrArray *fast_seek;
	void *fast_seek_cur;
};
typedef struct wtap_reader *FILE_T;

void close()
{
    terminating_ = true;

    // Update filter - Use:
    //     previous_filter if 'Close' (passed in follow() method)
    //     filter_out_filter_ if 'Filter Out This Stream' (built by appending !current_stream to previous_filter)
    //     leave filter alone if window closed. (current stream)
    emit updateFilter(output_filter_, TRUE);

    WiresharkDialog::close();
}
#define ws_close   close
void file_close(FILE_T file)
{
	int fd = file->fd;

	/* free memory and close file */
	if (file->size) {
#ifdef HAVE_ZLIB
		inflateEnd(&(file->strm));
#endif
		g_free(file->out.buf);
		g_free(file->in.buf);
	}
	g_free(file->fast_seek_cur);
	file->err = 0;
	file->err_info = NULL;
	g_free(file);
	/*
	* If fd is -1, somebody's done a file_closefd() on us, so
	* we don't need to close the FD itself, and shouldn't do
	* so.
	*/
	if (fd != -1)
		ws_close(fd);
}
#define ws_close   close

void wtap_sequential_close(wtap *wth)
{
	if (wth->subtype_sequential_close != NULL)
		(*wth->subtype_sequential_close)(wth);

	if (wth->fh != NULL) {
		file_close(wth->fh);
		wth->fh = NULL;
	}

	if (wth->rec_data) {
		ws_buffer_free(wth->rec_data);
		g_free(wth->rec_data);
		wth->rec_data = NULL;
	}
}
void
wtap_close(wtap *wth)
{
	wtap_sequential_close(wth);

	if (wth->subtype_close != NULL)
		(*wth->subtype_close)(wth);

	if (wth->random_fh != NULL)
		file_close(wth->random_fh);

	g_free(wth->priv);

	if (wth->fast_seek != NULL) {
		g_ptr_array_foreach(wth->fast_seek, g_fast_seek_item_free, NULL);
		g_ptr_array_free(wth->fast_seek, TRUE);
	}

	wtap_block_array_free(wth->shb_hdrs);
	wtap_block_array_free(wth->nrb_hdrs);
	wtap_block_array_free(wth->interface_data);

	g_free(wth);
}
void cf_close(capture_file *cf)
{
	cf->stop_flag = FALSE;
	if (cf->state == FILE_CLOSED)
		return; /* Nothing to do */

				/* Die if we're in the middle of reading a file. */
	g_assert(cf->state != FILE_READ_IN_PROGRESS);
	g_assert(!cf->read_lock);

	cf_callback_invoke(cf_cb_file_closing, cf);

	/* close things, if not already closed before */
	color_filters_cleanup();

	if (cf->provider.wth) {
		wtap_close(cf->provider.wth);
		cf->provider.wth = NULL;
	}
	/* We have no file open... */
	if (cf->filename != NULL) {
		/* If it's a temporary file, remove it. */
		if (cf->is_tempfile)
			ws_unlink(cf->filename);
		g_free(cf->filename);
		cf->filename = NULL;
	}
	/* ...which means we have no changes to that file to save. */
	cf->unsaved_changes = FALSE;

	/* no open_routine type */
	cf->open_type = WTAP_TYPE_AUTO;

	/* Clean up the record metadata. */
	wtap_rec_cleanup(&cf->rec);

	/* Free up the packet buffer. */
	ws_buffer_free(&cf->buf);

	dfilter_free(cf->rfcode);
	cf->rfcode = NULL;
	if (cf->provider.frames != NULL) {
		free_frame_data_sequence(cf->provider.frames);
		cf->provider.frames = NULL;
	}
	if (cf->provider.frames_user_comments) {
		g_tree_destroy(cf->provider.frames_user_comments);
		cf->provider.frames_user_comments = NULL;
	}
	cf_unselect_packet(cf);   /* nothing to select */
	cf->first_displayed = 0;
	cf->last_displayed = 0;

	/* No frames, no frame selected, no field in that frame selected. */
	cf->count = 0;
	cf->current_frame = 0;
	cf->current_row = 0;
	cf->finfo_selected = NULL;

	/* No frame link-layer types, either. */
	if (cf->linktypes != NULL) {
		g_array_free(cf->linktypes, TRUE);
		cf->linktypes = NULL;
	}

	/* Clear the packet list. */
	packet_list_freeze();
	packet_list_clear();
	packet_list_thaw();

	cf->f_datalen = 0;
	nstime_set_zero(&cf->elapsed_time);

	reset_tap_listeners();

	epan_free(cf->epan);
	cf->epan = NULL;

	/* We have no file open. */
	cf->state = FILE_CLOSED;

	cf_callback_invoke(cf_cb_file_closed, cf);
}
cf_status_t cf_open(capture_file *cf, const char *fname, unsigned int type, gboolean is_tempfile, int *err)
{
	wtap  *wth;
	gchar *err_info;

	wth = wtap_open_offline(fname, type, err, &err_info, TRUE);
	if (wth == NULL)
		goto fail;

	/* The open succeeded.  Close whatever capture file we had open,
	and fill in the information for this file. */
	cf_close(cf);

	/* Initialize the record metadata. */
	wtap_rec_init(&cf->rec);

	/* XXX - we really want to initialize this after we've read all
	the packets, so we know how much we'll ultimately need. */
	ws_buffer_init(&cf->buf, 1500);
	epan_free(cf->epan);
	/* We're about to start reading the file. */
	cf->state = FILE_READ_IN_PROGRESS;

	cf->provider.wth = wth;
	cf->f_datalen = 0;

	/* Set the file name because we need it to set the follow stream filter.
	XXX - is that still true?  We need it for other reasons, though,
	in any case. */
	cf->filename = g_strdup(fname);

	/* Indicate whether it's a permanent or temporary file. */
	cf->is_tempfile = is_tempfile;

	/* No user changes yet. */
	cf->unsaved_changes = FALSE;

	cf->computed_elapsed = 0;

	cf->cd_t = wtap_file_type_subtype(cf->provider.wth);
	cf->open_type = type;
	cf->linktypes = g_array_sized_new(FALSE, FALSE, (guint) sizeof(int), 1);
	cf->count = 0;
	cf->packet_comment_count = 0;
	cf->displayed_count = 0;
	cf->marked_count = 0;
	cf->ignored_count = 0;
	cf->ref_time_count = 0;
	cf->drops_known = FALSE;
	cf->drops = 0;
	cf->snap = wtap_snapshot_length(cf->provider.wth);

	/* Allocate a frame_data_sequence for the frames in this file */
	cf->provider.frames = new_frame_data_sequence();

	nstime_set_zero(&cf->elapsed_time);
	cf->provider.ref = NULL;
	cf->provider.prev_dis = NULL;
	cf->provider.prev_cap = NULL;
	cf->cum_bytes = 0;

	/* Create new epan session for dissection.
	* (The old one was freed in cf_close().)
	*/
	cf->epan = ws_epan_new(cf);

	packet_list_queue_draw();
	cf_callback_invoke(cf_cb_file_opened, cf);

	if (cf->cd_t == WTAP_FILE_TYPE_SUBTYPE_BER) {
		/* tell the BER dissector the file name */
		ber_set_filename(cf->filename);
	}

	wtap_set_cb_new_ipv4(cf->provider.wth, add_ipv4_name);
	wtap_set_cb_new_ipv6(cf->provider.wth, (wtap_new_ipv6_callback_t)add_ipv6_name);

	return CF_OK;

fail:
	//cfile_open_failure_alert_box(fname, *err, err_info);
	return CF_ERROR;
}

bool name_param;
bool ret = true;

bool openCaptureFile(char* cf_path/*, char* read_filter, unsigned int type, gboolean is_tempfile)*/)
{
	const char* file_name = "";
	//dfilter_t *rfcode = NULL;
	//gchar *err_msg;
	int err;
	//boolean name_param;
	//boolean ret = true;

	// was a file name given as function parameter?
	name_param = !strcmp(cf_path, "");

	for (;;) {

		/*if (cf_path.isEmpty()) {
			CaptureFileDialog open_dlg(this, capture_file_.capFile(), read_filter);

			if (open_dlg.open(file_name, type)) {
				cf_path = file_name;
			}
			else {
				ret = false;
				goto finish;
			}
		}*/

		// TODO detect call from "cf_read" -> "update_progress_dlg"
		// ("capture_file_.capFile()->read_lock"), possibly queue opening the
		// file and return early to avoid the warning in testCaptureFileClose.

		/*const char* before_what = " before opening another file";
		if (!testCaptureFileClose(before_what)) {
			ret = false;
			goto finish;
		}*/

		//if (dfilter_compile(qUtf8Printable(read_filter), &rfcode, &err_msg)) {
		//	cf_set_rfcode(CaptureFile::globalCapFile(), rfcode);
		//}
		//else {
		//	/* Not valid.  Tell the user, and go back and run the file
		//	selection box again once they dismiss the alert. */
		//	//bad_dfilter_alert_box(top_level, read_filter->str);
		//	QMessageBox::warning(this, tr("Invalid Display Filter"),
		//		char*("The filter expression ") +
		//		read_filter +
		//		char*(" isn't a valid display filter. (") +
		//		err_msg + char*(")."),
		//		QMessageBox::Ok);

		//	if (!name_param) {
		//		// go back to the selection dialogue only if the file
		//		// was selected from this dialogue
		//		cf_path.clear();
		//		continue;
		//	}
		//}

		///* Make the file name available via MainWindow */
		//setMwFileName(cf_path);

		/* Try to open the capture file. This closes the current file if it succeeds. */
		//CaptureFile::globalCapFile()->window = this;
		if (cf_open(CaptureFile::globalCapFile(), qUtf8Printable(cf_path), type, is_tempfile, &err) != CF_OK) {
			/* We couldn't open it; don't dismiss the open dialog box,
			just leave it around so that the user can, after they
			dismiss the alert box popped up for the open error,
			try again. */
			CaptureFile::globalCapFile()->window = NULL;
			//dfilter_free(rfcode);
			cf_path.clear();
			continue;
		}

		switch (cf_read(CaptureFile::globalCapFile(), FALSE)) {

		case CF_READ_OK:
		case CF_READ_ERROR:
			/* Just because we got an error, that doesn't mean we were unable
			to read any of the file; we handle what we could get from the
			file. */
			break;

		case CF_READ_ABORTED:
			/* The user bailed out of re-reading the capture file; the
			capture file has been closed - just free the capture file name
			string and return (without changing the last containing
			directory). */
			capture_file_.setCapFile(NULL);
			ret = false;
			goto finish;
		}
		break;
	}
	// get_dirname overwrites its path.
	wsApp->setLastOpenDir(get_dirname(cf_path.toUtf8().data()));

	main_ui_->statusBar->showExpert();

finish:
#ifdef HAVE_LIBPCAP
	if (global_commandline_info.quit_after_cap)
		exit(0);
#endif
	return ret;
}


int main()
{
    return 0;
}

