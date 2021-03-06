// extract wireshark source code.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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
union ieee_802_11_phy_info {
	struct ieee_802_11_fhss info_11_fhss;
	struct ieee_802_11b info_11b;
	struct ieee_802_11a info_11a;
	struct ieee_802_11g info_11g;
	struct ieee_802_11n info_11n;
	struct ieee_802_11ac info_11ac;
	struct ieee_802_11ad info_11ad;
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
struct wtap_erf_eth_hdr {
	guint8 offset;
	guint8 pad;
};
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

struct _frame_data_sequence {
	guint32      count;           /* Total number of frames */
	void        *ptree_root;      /* Pointer to the root node */
};
typedef struct _frame_data_sequence frame_data_sequence;

struct packet_provider_data {
	wtap        *wth;                  /* Wiretap session */
	const frame_data *ref;
	frame_data  *prev_dis;
	frame_data  *prev_cap;
	frame_data_sequence *frames;       /* Sequence of frames, if we're keeping that information */
	GTree       *frames_user_comments; /* BST with user comments for frames (key = frame_data) */
};


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
typedef struct tvbuff tvbuff_t;
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
typedef struct _e_guid_t {
	guint32 data1;
	guint16 data2;
	guint16 data3;
	guint8  data4[8];
} e_guid_t;
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
/* values for wtap_reader compression */
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

bool terminating_ = false;
#include <Winsock2.h>
//#define ws_close  close(fd)
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
		closesocket(fd);
}
#define SMALL_BUFFER_SIZE (2 * 1024) /* Everyone still uses 1500 byte frames, right? */

static GPtrArray *small_buffers = NULL; /* Guaranteed to be at least SMALL_BUFFER_SIZE */

void
ws_buffer_free(Buffer* buffer)
{
	g_assert(buffer);
	if (buffer->allocated == SMALL_BUFFER_SIZE) {
		g_ptr_array_add(small_buffers, buffer->data);
	}
	else {
		g_free(buffer->data);
	}
	buffer->data = NULL;
}
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
#define _U_
static void
g_fast_seek_item_free(gpointer data, gpointer user_data _U_)
{
	g_free(data);
}
/*
* Structure describing a type of block.
*/
typedef enum {
	WTAP_BLOCK_NG_SECTION = 0,
	WTAP_BLOCK_IF_DESCR,
	WTAP_BLOCK_NG_NRB,
	WTAP_BLOCK_IF_STATS,
	WTAP_BLOCK_END_OF_LIST
} wtap_block_type_t;
struct wtap_block;
typedef struct wtap_block *wtap_block_t;
typedef void(*wtap_block_create_func)(wtap_block_t block);

typedef void(*wtap_mand_free_func)(wtap_block_t block);
typedef void(*wtap_mand_copy_func)(wtap_block_t dest_block, wtap_block_t src_block);

typedef struct {
	wtap_block_type_t block_type;    /**< internal type code for block */
	const char *name;                /**< name of block */
	const char *description;         /**< human-readable description of block */
	wtap_block_create_func create;
	wtap_mand_free_func free_mand;
	wtap_mand_copy_func copy_mand;
	GArray *options;                 /**< array of known options */
} wtap_blocktype_t;

struct wtap_block
{
	wtap_blocktype_t* info;
	void* mandatory_data;
	GArray* options;
};







struct wtap_opttype_custom
{
	void* data;
	guint size;
};
typedef union {
	guint8 uint8val;
	guint64 uint64val;
	guint32 ipv4val;    /* network byte order */
	ws_in6_addr ipv6val;
	char *stringval;
	struct wtap_opttype_custom customval;
} wtap_optval_t;
typedef struct {
	guint option_id;     /**< option code for the option */
	wtap_optval_t value; /**< value */
} wtap_option_t;
typedef enum {
	WTAP_OPTTYPE_UINT8,
	WTAP_OPTTYPE_UINT64,
	WTAP_OPTTYPE_STRING,
	WTAP_OPTTYPE_IPv4,
	WTAP_OPTTYPE_IPv6,
	WTAP_OPTTYPE_CUSTOM
} wtap_opttype_e;
typedef void *(*wtap_opttype_dup_custom_func)(void* src);
typedef void(*wtap_opttype_free_custom_func)(void* data);
typedef struct {
	const char *name;                        /**< name of option */
	const char *description;                 /**< human-readable description of option */
	wtap_opttype_e data_type;                /**< data type of that option */
	guint flags;                             /**< flags for the option */
	wtap_opttype_dup_custom_func dup_func;   /**< function to duplicate custom option data */
	wtap_opttype_free_custom_func free_func; /**< function to free custom option data */
} wtap_opttype_t;
static void wtap_block_free_option(wtap_block_t block, wtap_option_t *opt)
{
	wtap_opttype_t *opttype;

	opttype = &g_array_index(block->info->options, wtap_opttype_t, opt->option_id);
	switch (opttype->data_type) {

	case WTAP_OPTTYPE_STRING:
		g_free(opt->value.stringval);
		break;

	case WTAP_OPTTYPE_CUSTOM:
		opttype->free_func(opt->value.customval.data);
		g_free(opt->value.customval.data);
		break;

	default:
		break;
	}
}
static void wtap_block_free_options(wtap_block_t block)
{
	guint i;
	wtap_option_t *opt;

	for (i = 0; i < block->options->len; i++) {
		opt = &g_array_index(block->options, wtap_option_t, i);
		wtap_block_free_option(block, opt);
	}
}
void wtap_block_free(wtap_block_t block)
{
	if (block != NULL)
	{
		if (block->info->free_mand != NULL)
			block->info->free_mand(block);

		g_free(block->mandatory_data);
		wtap_block_free_options(block);
		g_array_free(block->options, TRUE);
		g_free(block);
	}
}
void wtap_block_array_free(GArray* block_array)
{
	guint block;

	if (block_array == NULL)
		return;

	for (block = 0; block < block_array->len; block++) {
		wtap_block_free(g_array_index(block_array, wtap_block_t, block));
	}
	g_array_free(block_array, TRUE);
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
//#ifdef _WIN32
//int
//ws_stdio_unlink(const gchar *filename)
//{
//	wchar_t *wfilename = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
//	int retval;
//	int save_errno;
//
//	if (wfilename == NULL)
//	{
//		errno = EINVAL;
//		return -1;
//	}
//
//	retval = _wunlink(wfilename);
//	save_errno = errno;
//
//	g_free(wfilename);
//
//	errno = save_errno;
//	return retval;
//}
//#define ws_unlink	ws_stdio_unlink
//#else
#include <io.h>
#define ws_unlink		_unlink
//#endif
#define WTAP_TYPE_AUTO 0
void
wtap_rec_cleanup(wtap_rec *rec)
{
	ws_buffer_free(&rec->options_buf);
}

/*
* Free a frame_data_sequence and all the frame_data structures in it.
*/
#define NODES_PER_LEVEL         (1<<LOG2_NODES_PER_LEVEL)
#define LOG2_NODES_PER_LEVEL    10
void
frame_data_destroy(frame_data *fdata)
{
	if (fdata->pfd) {
		g_slist_free(fdata->pfd);
		fdata->pfd = NULL;
	}
}
/* recursively frees a frame_data radix level */

static void
free_frame_data_array(void *array, guint count, guint level, gboolean last)
{
	guint i, level_count;

	if (last) {
		/* if we are the last in our given parent's row, we may not have
		* exactly a full row, so do the bit twiddling to figure out exactly
		* how many fields we have */
		level_count = (count >> ((level - 1) * LOG2_NODES_PER_LEVEL)) &
			(NODES_PER_LEVEL - 1);
		/* the above calculation rounds down, so make sure we count correctly
		* if count is not an even multiple of NODES_PER_LEVEL */
		if (count & ((1 << ((level - 1) * LOG2_NODES_PER_LEVEL)) - 1)) {
			level_count++;
		}
	}
	else {
		/* if we're not the last in our parent, then we're guaranteed to have
		* a full array */
		level_count = NODES_PER_LEVEL;
	}


	if (level > 1) {
		/* recurse on every sub-array, passing on our own 'last' value
		* specially to our last child */
		frame_data **real_array = (frame_data **)array;

		for (i = 0; i < level_count - 1; i++) {
			free_frame_data_array(real_array[i], count, level - 1, FALSE);
		}

		free_frame_data_array(real_array[level_count - 1], count, level - 1, last);
	}
	else if (level == 1) {
		/* bottom level, so just clean up all the frame data */
		frame_data *real_array = (frame_data *)array;

		for (i = 0; i < level_count; i++) {
			frame_data_destroy(&real_array[i]);
		}
	}

	/* free the array itself */
	g_free(array);
}

void
free_frame_data_sequence(frame_data_sequence *fds)
{
	guint   levels;

	/* calculate how many levels we have */
	if (fds->count == 0) {
		/* The tree is empty; there are no levels. */
		levels = 0;
	}
	else if (fds->count <= NODES_PER_LEVEL) {
		/* It's a 1-level tree. */
		levels = 1;
	}
	else if (fds->count <= NODES_PER_LEVEL * NODES_PER_LEVEL) {
		/* It's a 2-level tree. */
		levels = 2;
	}
	else if (fds->count <= NODES_PER_LEVEL * NODES_PER_LEVEL*NODES_PER_LEVEL) {
		/* It's a 3-level tree. */
		levels = 3;
	}
	else {
		/* fds->count is 2^32-1 at most, and NODES_PER_LEVEL^4
		2^(LOG2_NODES_PER_LEVEL*4), and LOG2_NODES_PER_LEVEL is 10,
		so fds->count is always less < NODES_PER_LEVEL^4. */
		/* It's a 4-level tree. */
		levels = 4;
	}

	/* call the recursive free function */
	if (levels > 0) {
		free_frame_data_array(fds->ptree_root, fds->count, levels, TRUE);
	}

	/* free the header struct */
	g_free(fds);
}
/*
* Free up a frame's list of data sources.
*/
void
free_data_sources(packet_info *pinfo)
{
	if (pinfo->data_src) {
		g_slist_free(pinfo->data_src);
		pinfo->data_src = NULL;
	}
}
char *__crashreporter_info__ = NULL;
void
ws_vadd_crash_info(const char *fmt, va_list ap)
{
	char *m, *old_info, *new_info;

	m = g_strdup_vprintf(fmt, ap);
	if (__crashreporter_info__ == NULL)
		__crashreporter_info__ = m;
	else {
		old_info = __crashreporter_info__;
		new_info = g_strdup_printf("%s\n%s", old_info, m);
		g_free(m);
		__crashreporter_info__ = new_info;
		g_free(old_info);
	}
}
static void *(*allocator)(size_t) = (void *(*)(size_t)) g_malloc;
#define get_alloc() (allocator)
typedef struct {
	unsigned long except_group;
	unsigned long except_code;
} except_id_t;
typedef struct {
	except_id_t volatile except_id;
	const char *volatile except_message;
	void *volatile except_dyndata;
} except_t;
#define DissectorError		6 
#define _ATTRIBUTE(attrs)
void __assert_func(const char *, int, const char *, const char *) _ATTRIBUTE((__noreturn__));
# define assert(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, \
						       __ASSERT_FUNC, #__e))
#   define __ASSERT_FUNC ((char *) 0)

struct except_cleanup {
	void(*except_func)(void *);
	void *except_context;
};
#include <setjmp.h>
struct except_catch {
	except_id_t *except_id;
	size_t except_size;
	except_t except_obj;
	jmp_buf except_jmp;
};

enum except_stacktype {
	XCEPT_CLEANUP, XCEPT_CATCHER
};
struct except_stacknode {
	struct except_stacknode *except_down;
	enum except_stacktype except_type;
	union {
		struct except_catch *except_catcher;
		struct except_cleanup *except_cleanup;
	} except_info;
};
static struct except_stacknode *stack_top;
#define get_top() (stack_top)
static void(*deallocator)(void *) = g_free;
#define get_dealloc() (deallocator)
void except_free(void *ptr)
{
	get_dealloc()(ptr);
}
#define XCEPT_GROUP_ANY 0
#define XCEPT_CODE_ANY  0
#define XCEPT_BAD_ALLOC 1
static int match(const volatile except_id_t *thrown, const except_id_t *caught)
{
	int group_match = (caught->except_group == XCEPT_GROUP_ANY ||
		caught->except_group == thrown->except_group);
	int code_match = (caught->except_code == XCEPT_CODE_ANY ||
		caught->except_code == thrown->except_code);

	return group_match && code_match;
}
static void unhandled_catcher(except_t *except)
{
	if (except->except_message == NULL) {
		fprintf(stderr, "Unhandled exception (group=%lu, code=%lu)\n",
			except->except_id.except_group,
			except->except_id.except_code);
	}
	else {
		fprintf(stderr, "Unhandled exception (\"%s\", group=%lu, code=%lu)\n",
			except->except_message, except->except_id.except_group,
			except->except_id.except_code);
	}
	abort();
}

static void(*uh_catcher_ptr)(except_t *) = unhandled_catcher;
#define get_caget_catcher() (uh_catcher_ptr)
#define get_catcher() (uh_catcher_ptr)
#define set_top(T) (stack_top = (T))
static void do_throw(except_t *except)
{
    struct except_stacknode *top;

    assert (except->except_id.except_group != 0 &&
        except->except_id.except_code != 0);

    for (top = get_top(); top != 0; top = top->except_down) {
        if (top->except_type == XCEPT_CLEANUP) {
            top->except_info.except_cleanup->except_func(top->except_info.except_cleanup->except_context);
        } else {
            struct except_catch *catcher = top->except_info.except_catcher;
            const except_id_t *pi = catcher->except_id;
            size_t i;

            assert (top->except_type == XCEPT_CATCHER);
            except_free(catcher->except_obj.except_dyndata);

            for (i = 0; i < catcher->except_size; pi++, i++) {
                if (match(&except->except_id, pi)) {
                    //catcher->except_obj = *except;
                    set_top(top);
                    longjmp(catcher->except_jmp, 1);
                }
            }
        }
    }

    set_top(top);
    get_catcher()(except); /* unhandled exception */
    abort();
}
void except_throw(long group, long code, const char *msg)
{
	except_t except;

	except.except_id.except_group = group;
	except.except_id.except_code = code;
	except.except_message = msg;
	except.except_dyndata = 0;

#ifdef _WIN32
	if (code == DissectorError && IsDebuggerPresent()) {
		DebugBreak();
	}
#endif

	do_throw(&except);
}
void *except_alloc(size_t size)
{
	void *ptr = get_alloc()(size);

	if (ptr == 0)
		except_throw(XCEPT_BAD_ALLOC, 0, "out of memory");
	return ptr;
}
#define XCEPT_BUFFER_SIZE 1024
void except_throwd(long group, long code, const char *msg, void *data)
{
	except_t except;

	except.except_id.except_group = group;
	except.except_id.except_code = code;
	except.except_message = msg;
	except.except_dyndata = data;

	do_throw(&except);
}
void except_vthrowf(long group, long code, const char *fmt,
	va_list vl)
{
	char *buf = (char *)except_alloc(XCEPT_BUFFER_SIZE);

	g_vsnprintf(buf, XCEPT_BUFFER_SIZE, fmt, vl);
	except_throwd(group, code, buf, buf);
}
#define XCEPT_GROUP_WIRESHARK 1

#define VTHROW_FORMATTED(x, format, args) \
	except_vthrowf(XCEPT_GROUP_WIRESHARK, (x), format, args)
void proto_report_dissector_bug(const char *format, ...)
{
	va_list args;

	if (getenv("WIRESHARK_ABORT_ON_DISSECTOR_BUG") != NULL) {
		/*
		* Try to have the error message show up in the crash
		* information.
		*/
		va_start(args, format);
		ws_vadd_crash_info(format, args);
		va_end(args);

		/*
		* Print the error message.
		*/
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		putc('\n', stderr);

		/*
		* And crash.
		*/
		abort();
	}
	else {
		va_start(args, format);
		VTHROW_FORMATTED(DissectorError, format, args);
		va_end(args);
	}
}
#define REPORT_DISSECTOR_BUG(...)  \
	proto_report_dissector_bug(__VA_ARGS__)

#define __DISSECTOR_ASSERT_STRINGIFY(s)	# s
#define __DISSECTOR_ASSERT_STATIC_ANALYSIS_HINT(expression)
#define __DISSECTOR_ASSERT(expression, file, lineno)  \
  (REPORT_DISSECTOR_BUG("%s:%u: failed assertion \"%s\"", \
        file, lineno, __DISSECTOR_ASSERT_STRINGIFY(expression)))
#define DISSECTOR_ASSERT(expression)  \
  ((void) ((expression) ? (void)0 : \
   __DISSECTOR_ASSERT (expression, __FILE__, __LINE__))) \
   __DISSECTOR_ASSERT_STATIC_ANALYSIS_HINT(expression)
/** The pattern object used for ws_mempbrk_exec().
*/
typedef struct {
	gchar patt[256];
#ifdef HAVE_SSE4_2
	gboolean use_sse42;
	__m128i mask;
#endif
} ws_mempbrk_pattern;
struct tvb_ops {
	gsize tvb_size;
	void(*tvb_free)(struct tvbuff *tvb);
	guint(*tvb_offset)(const struct tvbuff *tvb, guint counter);
	const guint8 *(*tvb_get_ptr)(struct tvbuff *tvb, guint abs_offset, guint abs_length);
	void *(*tvb_memcpy)(struct tvbuff *tvb, void *target, guint offset, guint length);

	gint(*tvb_find_guint8)(tvbuff_t *tvb, guint abs_offset, guint limit, guint8 needle);
	gint(*tvb_ws_mempbrk_pattern_guint8)(tvbuff_t *tvb, guint abs_offset, guint limit, const ws_mempbrk_pattern* pattern, guchar *found_needle);

	tvbuff_t *(*tvb_clone)(tvbuff_t *tvb, guint abs_offset, guint abs_length);
};

typedef struct tvbuff tvbuff_t;
#define DISSECTOR_ASSERT(expression)  \
  ((void) ((expression) ? (void)0 : \
   __DISSECTOR_ASSERT (expression, __FILE__, __LINE__))) \
   __DISSECTOR_ASSERT_STATIC_ANALYSIS_HINT(expression)
static void
tvb_free_internal(tvbuff_t *tvb)
{
	gsize     size;

	DISSECTOR_ASSERT(tvb);

	if (tvb->ops->tvb_free)
		tvb->ops->tvb_free(tvb);

	size = tvb->ops->tvb_size;

	g_slice_free1(size, tvb);
}
void
tvb_free_chain(tvbuff_t  *tvb)
{
	tvbuff_t *next_tvb;
	DISSECTOR_ASSERT(tvb);
	while (tvb) {
		next_tvb = tvb->next;
		tvb_free_internal(tvb);
		tvb = next_tvb;
	}
}

/** A protocol tree element. */
typedef proto_node proto_tree;
#define PTREE_DATA(proto_tree)   ((proto_tree)->tree_data)
typedef void(*proto_tree_foreach_func)(proto_node *, gpointer);
void proto_tree_children_foreach(proto_tree *tree, proto_tree_foreach_func func,
	gpointer data)
{
	proto_node *node = tree;
	proto_node *current;

	if (!node)
		return;

	node = node->first_child;
	while (node != NULL) {
		current = node;
		node = current->next;
		func((proto_tree *)current, data);
	}
}

#define PNODE_FINFO(proto_node)  ((proto_node)->finfo)
static void
proto_tree_free_node(proto_node *node, gpointer data _U_)
{
	field_info *finfo = PNODE_FINFO(node);

	proto_tree_children_foreach(node, proto_tree_free_node, NULL);

	FVALUE_CLEANUP(&finfo->value);
}
typedef struct _gpa_hfinfo_t {
	guint32             len;
	guint32             allocated_len;
	header_field_info **hfi;
} gpa_hfinfo_t;
#define __DISSECTOR_ASSERT_HINT(expression, file, lineno, hint)  \
  (REPORT_DISSECTOR_BUG("%s:%u: failed assertion \"%s\" (%s)", \
        file, lineno, __DISSECTOR_ASSERT_STRINGIFY(expression), hint))

#define DISSECTOR_ASSERT_HINT(expression, hint)  \
  ((void) ((expression) ? (void)0 : \
   __DISSECTOR_ASSERT_HINT (expression, __FILE__, __LINE__, hint))) \
   __DISSECTOR_ASSERT_STATIC_ANALYSIS_HINT(expression)

static gpa_hfinfo_t gpa_hfinfo;
#define PROTO_REGISTRAR_GET_NTH(hfindex, hfinfo)						\
	if((guint)hfindex >= gpa_hfinfo.len && getenv("WIRESHARK_ABORT_ON_DISSECTOR_BUG"))	\
		g_error("Unregistered hf! index=%d", hfindex);					\
	DISSECTOR_ASSERT_HINT((guint)hfindex < gpa_hfinfo.len, "Unregistered hf!");	\
	DISSECTOR_ASSERT_HINT(gpa_hfinfo.hfi[hfindex] != NULL, "Unregistered hf!");	\
	hfinfo = gpa_hfinfo.hfi[hfindex];

static void
free_GPtrArray_value(gpointer key, gpointer value, gpointer user_data _U_)
{
	GPtrArray         *ptrs = (GPtrArray *)value;
	gint               hfid = GPOINTER_TO_UINT(key);
	header_field_info *hfinfo;

	PROTO_REGISTRAR_GET_NTH(hfid, hfinfo);
	if (hfinfo->ref_type != HF_REF_TYPE_NONE) {
		/* when a field is referenced by a filter this also
		affects the refcount for the parent protocol so we need
		to adjust the refcount for the parent as well
		*/
		if (hfinfo->parent != -1) {
			header_field_info *parent_hfinfo;
			PROTO_REGISTRAR_GET_NTH(hfinfo->parent, parent_hfinfo);
			parent_hfinfo->ref_type = HF_REF_TYPE_NONE;
		}
		hfinfo->ref_type = HF_REF_TYPE_NONE;
	}

	g_ptr_array_free(ptrs, TRUE);
}
/* frees the resources that the dissection a proto_tree uses */
void
proto_tree_free(proto_tree *tree)
{
	tree_data_t *tree_data = PTREE_DATA(tree);

	proto_tree_children_foreach(tree, proto_tree_free_node, NULL);

	/* free tree data */
	if (tree_data->interesting_hfids) {
		/* Free all the GPtrArray's in the interesting_hfids hash. */
		g_hash_table_foreach(tree_data->interesting_hfids,
			free_GPtrArray_value, NULL);

		/* And then destroy the hash. */
		g_hash_table_destroy(tree_data->interesting_hfids);
	}

	g_slice_free(tree_data_t, tree_data);

	g_slice_free(proto_tree, tree);
}

typedef enum _wmem_cb_event_t {
	WMEM_CB_FREE_EVENT,    /**< wmem_free_all() */
	WMEM_CB_DESTROY_EVENT  /**< wmem_destroy_allocator() */
} wmem_cb_event_t;
typedef gboolean(*wmem_user_cb_t) (wmem_allocator_t*, wmem_cb_event_t, void*);
typedef struct _wmem_user_cb_container_t {
	wmem_user_cb_t                    cb;
	void                             *user_data;
	struct _wmem_user_cb_container_t *next;
	guint                             id;
} wmem_user_cb_container_t;
void
wmem_free(wmem_allocator_t *allocator, void *ptr)
{
	if (allocator == NULL) {
		g_free(ptr);
		return;
	}

	g_assert(allocator->in_scope);

	if (ptr == NULL) {
		return;
	}

	allocator->wfree(allocator->private_data, ptr);
}
void
wmem_call_callbacks(wmem_allocator_t *allocator, wmem_cb_event_t event)
{
	wmem_user_cb_container_t **prev, *cur;
	gboolean again;

	prev = &(allocator->callbacks);
	cur = allocator->callbacks;

	while (cur) {

		/* call it */
		again = cur->cb(allocator, event, cur->user_data);

		/* if the callback requested deregistration, or this is being triggered
		* by the final destruction of the allocator, remove the callback */
		if (!again || event == WMEM_CB_DESTROY_EVENT) {
			*prev = cur->next;
			wmem_free(NULL, cur);
			cur = *prev;
		}
		else {
			prev = &(cur->next);
			cur = cur->next;
		}
	}
}
static void
wmem_free_all_real(wmem_allocator_t *allocator, gboolean final)
{
	wmem_call_callbacks(allocator,
		final ? WMEM_CB_DESTROY_EVENT : WMEM_CB_FREE_EVENT);
	allocator->free_all(allocator->private_data);
}

void
wmem_free_all(wmem_allocator_t *allocator)
{
	wmem_free_all_real(allocator, FALSE);
}
void
wmem_destroy_allocator(wmem_allocator_t *allocator)
{

	wmem_free_all_real(allocator, TRUE);
	allocator->cleanup(allocator->private_data);
	wmem_free(NULL, allocator);
}
static wmem_allocator_t *pinfo_pool_cache = NULL;
void
epan_dissect_cleanup(epan_dissect_t* edt)
{
	g_assert(edt);

#ifdef HAVE_PLUGINS
	g_slist_foreach(epan_plugins, epan_plugin_dissect_cleanup, edt);
#endif

	g_slist_free(edt->pi.proto_data);
	g_slist_free(edt->pi.dependent_frames);

	/* Free the data sources list. */
	free_data_sources(&edt->pi);

	if (edt->tvb) {
		/* Free all tvb's chained from this tvb */
		tvb_free_chain(edt->tvb);
	}

	if (edt->tree) {
		proto_tree_free(edt->tree);
	}

	if (pinfo_pool_cache == NULL) {
		wmem_free_all(edt->pi.pool);
		pinfo_pool_cache = edt->pi.pool;
	}
	else {
		wmem_destroy_allocator(edt->pi.pool);
	}
}

void
epan_dissect_free(epan_dissect_t* edt)
{
	epan_dissect_cleanup(edt);
	g_free(edt);
}
/* Unselect the selected packet, if any. */
void
cf_unselect_packet(capture_file *cf)
{
	epan_dissect_t *old_edt = cf->edt;

	cf->edt = NULL;

	/* No packet is selected. */
	cf->current_frame = NULL;
	cf->current_row = 0;

	/* Destroy the epan_dissect_t for the unselected packet. */
	if (old_edt != NULL)
		epan_dissect_free(old_edt);
}
//static PacketList *gbl_cur_packet_list = NULL;
//void
//packet_list_freeze(void)
//{
//	if (gbl_cur_packet_list) {
//		gbl_cur_packet_list->freeze();
//	}
//}

//void
//packet_list_thaw(void)
//{
//	if (gbl_cur_packet_list) {
//		gbl_cur_packet_list->thaw();
//	}
//
//	packets_bar_update();
//}
void nstime_set_zero(nstime_t *nstime)
{
	nstime->secs = 0;
	nstime->nsecs = 0;
}
typedef void(*tap_reset_cb)(void *tapdata);
typedef gboolean(*tap_packet_cb)(void *tapdata, packet_info *pinfo, epan_dissect_t *edt, const void *data);
typedef void(*tap_draw_cb)(void *tapdata);
typedef void(*tap_finish_cb)(void *tapdata);
typedef struct _tap_listener_t {
	struct _tap_listener_t *next;
	int tap_id;
	gboolean needs_redraw;
	guint flags;
	gchar *fstring;
	dfilter_t *code;
	void *tapdata;
	tap_reset_cb reset;
	tap_packet_cb packet;
	tap_draw_cb draw;
	tap_finish_cb finish;
} tap_listener_t;
static tap_listener_t *tap_listener_queue = NULL;
/* This function is called when we need to reset all tap listeners, for example
when we open/start a new capture or if we need to rescan the packet list.
*/
void
reset_tap_listeners(void)
{
	tap_listener_t *tl;

	for (tl = tap_listener_queue; tl; tl = tl->next) {
		if (tl->reset) {
			tl->reset(tl->tapdata);
		}
		tl->needs_redraw = TRUE;
	}

}
static GSList *cleanup_routines = NULL;
typedef void(*void_func_t)(void);
static void
call_routine(gpointer routine, gpointer dummy _U_)
{
	void_func_t func = (void_func_t)routine;
	(*func)();
}
void
expert_packet_cleanup(void)
{
	//đù trong này thực sự ko có gì  :))
}

/* the hash table */
static GHashTable *stream_hash;


/* cleanup reset function, call from stream_cleanup() */
static void cleanup_stream_hash(void) {
	if (stream_hash != NULL) {
		g_hash_table_destroy(stream_hash);
		stream_hash = NULL;
	}
}
/* the hash table */
static GHashTable *fragment_hash;


/* cleanup function, call from stream_cleanup() */
static void cleanup_fragment_hash(void) {
	if (fragment_hash != NULL) {
		g_hash_table_destroy(fragment_hash);
		fragment_hash = NULL;
	}
}
static void stream_cleanup_pdu_data(void)
{
	// đù trong này cg ko có gì luôn :)))
}
/*
* Generates a fragment identifier based on the given parameters. "data" is an
* opaque type whose interpretation is up to the caller of fragment_add*
* functions and the fragment key function (possibly NULL if you do not care).
*
* Keys returned by this function are only used within this packet scope.
*/
typedef gpointer(*fragment_temporary_key)(const packet_info *pinfo,
	const guint32 id, const void *data);

/*
* Like fragment_temporary_key, but used for identifying reassembled fragments
* which may persist through multiple packets.
*/
typedef gpointer(*fragment_persistent_key)(const packet_info *pinfo,
	const guint32 id, const void *data);

/*
* Data structure to keep track of fragments and reassemblies.
*/
typedef struct {
	GHashTable *fragment_table;
	GHashTable *reassembled_table;
	fragment_temporary_key temporary_key_func;
	fragment_persistent_key persistent_key_func;
	GDestroyNotify free_temporary_key_func;		/* temporary key destruction function */
} reassembly_table;
typedef struct _fragment_item {
	struct _fragment_item *next;
	guint32 frame;			/* XXX - does this apply to reassembly heads? */
	guint32	offset;			/* XXX - does this apply to reassembly heads? */
	guint32	len;			/* XXX - does this apply to reassembly heads? */
	guint32 fragment_nr_offset;	/**< offset for frame numbering, for sequences, where the
								* provided fragment number of the first fragment does
								* not start with 0
								* XXX - does this apply only to reassembly heads? */
	guint32 datalen;		/**< When flags&FD_BLOCKSEQUENCE is set, the
							* index of the last block (segments in
							* datagram + 1); otherwise the number of
							* bytes of the full datagram. Only valid in
							* the first item of the fragments list when
							* flags&FD_DATALEN is set.*/
	guint32 reassembled_in;		/**< frame where this PDU was reassembled,
								* only valid in the first item of the list
								* and when FD_DEFRAGMENTED is set*/
	guint8 reas_in_layer_num;	/**< The current "depth" or layer number in the current frame where reassembly was completed.
								* Example: in SCTP there can be several data chunks and we want the reassemblied tvb for the final
								* segment only. */
	guint32 flags;			/**< XXX - do some of these apply only to reassembly
							* heads and others only to fragments within
							* a reassembly? */
	tvbuff_t *tvb_data;
	/**
	* Null if the reassembly had no error; non-null if it had
	* an error, in which case it's the string for the error.
	*
	* XXX - this is wasted in all but the reassembly head; we
	* should probably have separate data structures for a
	* reassembly and for the fragments in a reassembly.
	*/
	const char *error;
} fragment_item, fragment_head;
#define FD_SUBSET_TVB           0x0020
/* XXX: just call tvb_free_chain();
*      Not removed so that existing dissectors using tvb_free() need not be changed.
*      I'd argue that existing calls to tvb_free() should have actually beeen
*      calls to tvb_free_chain() although the calls were OK as long as no
*      subsets, etc had been created on the tvb. */
void
tvb_free(tvbuff_t *tvb)
{
	tvb_free_chain(tvb);
}
/*
* For a fragment hash table entry, free the associated fragments.
* The entry value (fd_chain) is freed herein and the entry is freed
* when the key freeing routine is called (as a consequence of returning
* TRUE from this function).
*/
static gboolean
free_all_fragments(gpointer key_arg _U_, gpointer value, gpointer user_data _U_)
{
	fragment_head *fd_head;
	fragment_item *tmp_fd;

	/* g_hash_table_new_full() was used to supply a function
	* to free the key and anything to which it points
	*/
	for (fd_head = (fragment_head *)value; fd_head != NULL; fd_head = tmp_fd) {
		tmp_fd = fd_head->next;

		if (fd_head->tvb_data && !(fd_head->flags&FD_SUBSET_TVB))
			tvb_free(fd_head->tvb_data);
		g_slice_free(fragment_item, fd_head);
	}

	return TRUE;
}
#define FD_VISITED_FREE 0xffff

/*
* For a reassembled-packet hash table entry, free the fragment data
* to which the value refers and also the key itself.
*/
static gboolean
free_all_reassembled_fragments(gpointer key_arg _U_, gpointer value,
	gpointer user_data)
{
	GPtrArray *allocated_fragments = (GPtrArray *)user_data;
	fragment_head *fd_head;

	for (fd_head = (fragment_head *)value; fd_head != NULL; fd_head = fd_head->next) {
		/*
		* A reassembled packet is inserted into the
		* hash table once for every frame that made
		* up the reassembled packet; add first seen
		* fragments to array and later free them in
		* free_fragments()
		*/
		if (fd_head->flags != FD_VISITED_FREE) {
			if (fd_head->flags & FD_SUBSET_TVB)
				fd_head->tvb_data = NULL;
			g_ptr_array_add(allocated_fragments, fd_head);
			fd_head->flags = FD_VISITED_FREE;
		}
	}

	return TRUE;
}
static void
free_fragments(gpointer data, gpointer user_data _U_)
{
	fragment_item *fd_head = (fragment_item *)data;

	if (fd_head->tvb_data)
		tvb_free(fd_head->tvb_data);
	g_slice_free(fragment_item, fd_head);
}
/*
* Destroy a reassembly table.
*/
void reassembly_table_destroy(reassembly_table *table)
{
	/*
	* Clear the function pointers.
	*/
	table->temporary_key_func = NULL;
	table->persistent_key_func = NULL;
	table->free_temporary_key_func = NULL;
	if (table->fragment_table != NULL) {
		/*
		* The fragment hash table exists.
		*
		* Remove all entries and free fragment data for each entry.
		*
		* The keys, and anything to which they point, are freed by
		* calling the table's key freeing function.  The values
		* are freed in free_all_fragments().
		*/
		g_hash_table_foreach_remove(table->fragment_table,
			free_all_fragments, NULL);

		/*
		* Now destroy the hash table.
		*/
		g_hash_table_destroy(table->fragment_table);
		table->fragment_table = NULL;
	}
	if (table->reassembled_table != NULL) {
		GPtrArray *allocated_fragments;

		/*
		* The reassembled-packet hash table exists.
		*
		* Remove all entries and free reassembled packet
		* data and key for each entry.
		*/

		allocated_fragments = g_ptr_array_new();
		g_hash_table_foreach_remove(table->reassembled_table,
			free_all_reassembled_fragments, allocated_fragments);

		g_ptr_array_foreach(allocated_fragments, free_fragments, NULL);
		g_ptr_array_free(allocated_fragments, TRUE);

		/*
		* Now destroy the hash table.
		*/
		g_hash_table_destroy(table->reassembled_table);
		table->reassembled_table = NULL;
	}
}
static reassembly_table stream_reassembly_table;
/* cleanup the stream routines */
/* Note: stream_cleanup must only be called when seasonal memory
*       is also freed since the hash tables countain pointers to
*       wmem_file_scoped memory.
*/
void stream_cleanup(void)
{
	cleanup_stream_hash();
	cleanup_fragment_hash();
	stream_cleanup_pdu_data();
	reassembly_table_destroy(&stream_reassembly_table);
}
static wmem_allocator_t *file_scope = NULL;
static wmem_allocator_t *packet_scope = NULL;
void
wmem_gc(wmem_allocator_t *allocator)
{
	allocator->gc(allocator->private_data);
}
void
wmem_leave_file_scope(void)
{
	g_assert(file_scope);
	g_assert(file_scope->in_scope);
	g_assert(!packet_scope->in_scope);

	wmem_free_all(file_scope);
	file_scope->in_scope = FALSE;

	/* this seems like a good time to do garbage collection */
	wmem_gc(file_scope);
	wmem_gc(packet_scope);
}

void
cleanup_dissection(void)
{
	/* Cleanup protocol-specific variables. */
	g_slist_foreach(cleanup_routines, &call_routine, NULL);

	/* Cleanup the stream-handling tables */
	stream_cleanup();

	/* Cleanup the expert infos */
	expert_packet_cleanup();

	wmem_leave_file_scope();

	/*
	* Keep the name resolution info around until we start the next
	* dissection. Lua scripts may potentially do name resolution at
	* any time, even if we're not dissecting and have no capture
	* file open.
	*/
}
void
epan_free(epan_t *session)
{
	if (session) {
		/* XXX, it should take session as param */
		cleanup_dissection();

		g_slice_free(epan_t, session);
	}
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
	//packet_list_freeze();
	//packet_list_clear();
	//packet_list_thaw();

	cf->f_datalen = 0;
	nstime_set_zero(&cf->elapsed_time);

	reset_tap_listeners();

	epan_free(cf->epan);
	cf->epan = NULL;

	/* We have no file open. */
	cf->state = FILE_CLOSED;

	cf_callback_invoke(cf_cb_file_closed, cf);
}
/* Initializes a buffer with a certain amount of allocated space */
void
ws_buffer_init(Buffer* buffer, gsize space)
{
	g_assert(buffer);
	if (G_UNLIKELY(!small_buffers)) small_buffers = g_ptr_array_sized_new(1024);

	if (space <= SMALL_BUFFER_SIZE) {
		if (small_buffers->len > 0) {
			buffer->data = (guint8*)g_ptr_array_remove_index(small_buffers, small_buffers->len - 1);
		}
		else {
			buffer->data = (guint8*)g_malloc(SMALL_BUFFER_SIZE);
		}
		buffer->allocated = SMALL_BUFFER_SIZE;
	}
	else {
		buffer->data = (guint8*)g_malloc(space);
		buffer->allocated = space;
	}
	buffer->start = 0;
	buffer->first_free = 0;
}
void
wtap_rec_init(wtap_rec *rec)
{
	memset(rec, 0, sizeof *rec);
	ws_buffer_init(&rec->options_buf, 0);
}
int
wtap_file_type_subtype(wtap *wth)
{
	return wth->file_type_subtype;
}
guint
wtap_snapshot_length(wtap *wth)
{
	return wth->snapshot_length;
}
frame_data_sequence *
new_frame_data_sequence(void)
{
	frame_data_sequence *fds;

	fds = (frame_data_sequence *)g_malloc(sizeof *fds);
	fds->count = 0;
	fds->ptree_root = NULL;
	return fds;
}
#define LEAF_INDEX(framenum) \
        (((framenum) >> (0*LOG2_NODES_PER_LEVEL)) & (NODES_PER_LEVEL - 1))

/*
* Find the frame_data for the specified frame number.
*/
frame_data *
frame_data_sequence_find(frame_data_sequence *fds, guint32 num)
{
	frame_data *leaf;
	frame_data **level1;
	frame_data ***level2;
	frame_data ****level3;

	if (num == 0) {
		/* There is no frame number 0 */
		return NULL;
	}

	/* Convert it into an index number. */
	num--;
	if (num >= fds->count) {
		/* There aren't that many frames. */
		return NULL;
	}

	if (fds->count <= NODES_PER_LEVEL) {
		/* It's a 1-level tree. */
		leaf = (frame_data *)fds->ptree_root;
		return &leaf[num];
	}
	if (fds->count <= NODES_PER_LEVEL * NODES_PER_LEVEL) {
		/* It's a 2-level tree. */
		level1 = (frame_data **)fds->ptree_root;
		leaf = level1[num >> LOG2_NODES_PER_LEVEL];
		return &leaf[LEAF_INDEX(num)];
	}
	if (fds->count <= NODES_PER_LEVEL * NODES_PER_LEVEL*NODES_PER_LEVEL) {
		/* It's a 3-level tree. */
		level2 = (frame_data ***)fds->ptree_root;
		level1 = level2[num >> (LOG2_NODES_PER_LEVEL + LOG2_NODES_PER_LEVEL)];
		leaf = level1[(num >> LOG2_NODES_PER_LEVEL) & (NODES_PER_LEVEL - 1)];
		return &leaf[LEAF_INDEX(num)];
	}
	/* fds->count is 2^32-1 at most, and NODES_PER_LEVEL^4
	2^(LOG2_NODES_PER_LEVEL*4), and LOG2_NODES_PER_LEVEL is 10,
	so fds->count is always less < NODES_PER_LEVEL^4. */
	/* It's a 4-level tree, and is going to stay that way forever. */
	level3 = (frame_data ****)fds->ptree_root;
	level2 = level3[num >> (LOG2_NODES_PER_LEVEL + LOG2_NODES_PER_LEVEL + LOG2_NODES_PER_LEVEL)];
	level1 = level2[(num >> (LOG2_NODES_PER_LEVEL + LOG2_NODES_PER_LEVEL)) & (NODES_PER_LEVEL - 1)];
	leaf = level1[(num >> LOG2_NODES_PER_LEVEL) & (NODES_PER_LEVEL - 1)];
	return &leaf[LEAF_INDEX(num)];
}
static const nstime_t *
ws_get_frame_ts(struct packet_provider_data *prov, guint32 frame_num)
{
	if (prov->prev_dis && prov->prev_dis->num == frame_num)
		return &prov->prev_dis->abs_ts;

	if (prov->prev_cap && prov->prev_cap->num == frame_num)
		return &prov->prev_cap->abs_ts;

	if (prov->frames) {
		frame_data *fd = frame_data_sequence_find(prov->frames, frame_num);

		return (fd) ? &fd->abs_ts : NULL;
	}

	return NULL;
}
/** struct holding the information to build IDB:s
*  the interface_data array holds an array of wtap_block_t
*  represending IDB of one per interface.
*/
typedef struct wtapng_iface_descriptions_s {
	GArray *interface_data;
} wtapng_iface_descriptions_t;
wtapng_iface_descriptions_t *
wtap_file_get_idb_info(wtap *wth)
{
	wtapng_iface_descriptions_t *idb_info;

	idb_info = g_new(wtapng_iface_descriptions_t, 1);

	idb_info->interface_data = wth->interface_data;

	return idb_info;
}
typedef enum {
	WTAP_OPTTYPE_SUCCESS = 0,
	WTAP_OPTTYPE_NO_SUCH_OPTION = -1,
	WTAP_OPTTYPE_NOT_FOUND = -2,
	WTAP_OPTTYPE_TYPE_MISMATCH = -3,
	WTAP_OPTTYPE_NUMBER_MISMATCH = -4,
	WTAP_OPTTYPE_ALREADY_EXISTS = -5
} wtap_opttype_return_val;
#define WTAP_OPTTYPE_FLAG_MULTIPLE_ALLOWED 0x00000001 /* multiple instances allowed */
static wtap_optval_t * wtap_block_get_option(wtap_block_t block, guint option_id)
{
	guint i;
	wtap_option_t *opt;

	for (i = 0; i < block->options->len; i++) {
		opt = &g_array_index(block->options, wtap_option_t, i);
		if (opt->option_id == option_id)
			return &opt->value;
	}

	return NULL;
}
static wtap_opttype_return_val
wtap_block_get_option_common(wtap_block_t block, guint option_id, wtap_opttype_e type, wtap_optval_t **optvalp)
{
	wtap_opttype_t *opttype;
	wtap_optval_t *optval;

	if (option_id >= block->info->options->len) {
		/* There's no option for this block with that option ID */
		return WTAP_OPTTYPE_NO_SUCH_OPTION;
	}

	opttype = &g_array_index(block->info->options, wtap_opttype_t, option_id);

	/*
	* Is this an option of the specified data type?
	*/
	if (opttype->data_type != type) {
		/*
		* No.
		*/
		return WTAP_OPTTYPE_TYPE_MISMATCH;
	}

	/*
	* Can there be more than one instance of this option?
	*/
	if (opttype->flags & WTAP_OPTTYPE_FLAG_MULTIPLE_ALLOWED) {
		/*
		* Yes.  You can't ask for "the" value.
		*/
		return WTAP_OPTTYPE_NUMBER_MISMATCH;
	}

	optval = wtap_block_get_option(block, option_id);
	if (optval == NULL) {
		/* Didn't find the option */
		return WTAP_OPTTYPE_NOT_FOUND;
	}

	*optvalp = optval;
	return WTAP_OPTTYPE_SUCCESS;
}
wtap_opttype_return_val
wtap_block_get_string_option_value(wtap_block_t block, guint option_id, char** value)
{
	wtap_opttype_return_val ret;
	wtap_optval_t *optval;

	ret = wtap_block_get_option_common(block, option_id, WTAP_OPTTYPE_STRING, &optval);
	if (ret != WTAP_OPTTYPE_SUCCESS)
		return ret;
	*value = optval->stringval;
	return WTAP_OPTTYPE_SUCCESS;
}
#define OPT_IDB_NAME         2     /**< A UTF-8 string containing the name
*     of the device used to capture data.
*     "eth0" / "\Device\NPF_{AD1CE675-96D0-47C5-ADD0-2504B9126B68}"
*/
#define OPT_IDB_DESCR        3     /**< A UTF-8 string containing the description
*     of the device used to capture data.
*     "Wi-Fi" / "Local Area Connection" /
*     "Wireless Network Connection" /
*     "First Ethernet Interface"
*/
#define OPT_IDB_HARDWARE     15     /**< A UTF-8 string containing the description
*     of the hardware of the device used
*     to capture data.
*     "Broadcom NetXtreme" /
*     "Intel(R) PRO/1000 MT Network Connection" /
*     "NETGEAR WNA1000Mv2 N150 Wireless USB Micro Adapter"
*/
const char *
cap_file_provider_get_interface_name(struct packet_provider_data *prov, guint32 interface_id)
{
	wtapng_iface_descriptions_t *idb_info;
	wtap_block_t wtapng_if_descr = NULL;
	char* interface_name;

	idb_info = wtap_file_get_idb_info(prov->wth);

	if (interface_id < idb_info->interface_data->len)
		wtapng_if_descr = g_array_index(idb_info->interface_data, wtap_block_t, interface_id);

	g_free(idb_info);

	if (wtapng_if_descr) {
		if (wtap_block_get_string_option_value(wtapng_if_descr, OPT_IDB_NAME, &interface_name) == WTAP_OPTTYPE_SUCCESS)
			return interface_name;
		if (wtap_block_get_string_option_value(wtapng_if_descr, OPT_IDB_DESCR, &interface_name) == WTAP_OPTTYPE_SUCCESS)
			return interface_name;
		if (wtap_block_get_string_option_value(wtapng_if_descr, OPT_IDB_HARDWARE, &interface_name) == WTAP_OPTTYPE_SUCCESS)
			return interface_name;
	}
	return "unknown";
}

const char *
cap_file_provider_get_interface_description(struct packet_provider_data *prov, guint32 interface_id)
{
	wtapng_iface_descriptions_t *idb_info;
	wtap_block_t wtapng_if_descr = NULL;
	char* interface_name;

	idb_info = wtap_file_get_idb_info(prov->wth);

	if (interface_id < idb_info->interface_data->len)
		wtapng_if_descr = g_array_index(idb_info->interface_data, wtap_block_t, interface_id);

	g_free(idb_info);

	if (wtapng_if_descr) {
		if (wtap_block_get_string_option_value(wtapng_if_descr, OPT_IDB_DESCR, &interface_name) == WTAP_OPTTYPE_SUCCESS)
			return interface_name;
	}
	return NULL;
}

const char *
cap_file_provider_get_user_comment(struct packet_provider_data *prov, const frame_data *fd)
{
	if (prov->frames_user_comments)
		return (const char *)g_tree_lookup(prov->frames_user_comments, fd);

	/* g_warning? */
	return NULL;
}
#define MAXNAMELEN  	64	/* max name length (hostname and port name) */

typedef struct sub_net_hashipv4 {
	guint             addr;
	/* XXX: No longer needed?*/
	guint8            flags;          /* B0 dummy_entry, B1 resolve, B2 If the address is used in the trace */
	struct sub_net_hashipv4   *next;
	gchar             name[MAXNAMELEN];
} sub_net_hashipv4_t;
typedef struct _wmem_map_item_t {
	const void *key;
	void *value;
	struct _wmem_map_item_t *next;
} wmem_map_item_t;
struct _wmem_map_t {
	guint count; /* number of items stored */

				 /* The base-2 logarithm of the actual size of the table. We store this
				 * value for efficiency in hashing, since finding the actual capacity
				 * becomes just a left-shift (see the CAPACITY macro) whereas taking
				 * logarithms is expensive. */
	size_t capacity;

	wmem_map_item_t **table;

	GHashFunc  hash_func;
	GEqualFunc eql_func;

	guint      master_cb_id;
	guint      slave_cb_id;

	wmem_allocator_t *master;
	wmem_allocator_t *allocator;
};
typedef struct _wmem_map_t wmem_map_t;
static void
_host_name_lookup_cleanup(void) {
}
static wmem_map_t *ipxnet_hash_table = NULL;
static wmem_map_t *ipv4_hash_table = NULL;
static wmem_map_t *ipv6_hash_table = NULL;
//static wmem_map_t *vlan_hash_table = NULL;
static wmem_map_t *ss7pc_hash_table = NULL;
#define SUBNETLENGTHSIZE   32  /*1-32 inc.*/
typedef struct {
	gsize        mask_length;      /*1-32*/
	guint32      mask;             /* e.g. 255.255.255.*/
	sub_net_hashipv4_t** subnet_addresses; /* Hash table of subnet addresses */
} subnet_length_entry_t;
static subnet_length_entry_t subnet_length_entries[SUBNETLENGTHSIZE]; /* Ordered array of entries */
#define HASHHOSTSIZE     2048
static wmem_allocator_t *epan_scope = NULL;
/* Epan Scope */
wmem_allocator_t *
wmem_epan_scope(void)
{
	g_assert(epan_scope);

	return epan_scope;
}
static gboolean have_subnet_entry = FALSE;

static gboolean new_resolved_objects = FALSE;
static void host_name_lookup_cleanup(void)
{
	guint32 i, j;
	sub_net_hashipv4_t *entry, *next_entry;

	_host_name_lookup_cleanup();

	ipxnet_hash_table = NULL;
	ipv4_hash_table = NULL;
	ipv6_hash_table = NULL;
	ss7pc_hash_table = NULL;

	for (i = 0; i < SUBNETLENGTHSIZE; ++i) {
		if (subnet_length_entries[i].subnet_addresses != NULL) {
			for (j = 0; j < HASHHOSTSIZE; j++) {
				for (entry = subnet_length_entries[i].subnet_addresses[j];
					entry != NULL; entry = next_entry) {
					next_entry = entry->next;
					wmem_free(wmem_epan_scope(), entry);
				}
			}
			wmem_free(wmem_epan_scope(), subnet_length_entries[i].subnet_addresses);
			subnet_length_entries[i].subnet_addresses = NULL;
		}
	}

	have_subnet_entry = FALSE;
	new_resolved_objects = FALSE;
}
void *
wmem_alloc(wmem_allocator_t *allocator, const size_t size)
{
	if (allocator == NULL) {
		return g_malloc(size);
	}

	g_assert(allocator->in_scope);

	if (size == 0) {
		return NULL;
	}

	return allocator->walloc(allocator->private_data, size);
}
#define wmem_new(allocator, type) \
    ((type*)wmem_alloc((allocator), sizeof(type)))

wmem_map_t *
wmem_map_new(wmem_allocator_t *allocator,
	GHashFunc hash_func, GEqualFunc eql_func)
{
	wmem_map_t *map;

	map = wmem_new(allocator, wmem_map_t);

	map->hash_func = hash_func;
	map->eql_func = eql_func;
	map->master = allocator;
	map->allocator = allocator;
	map->count = 0;
	map->table = NULL;

	return map;
}
/* http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx#existing
* One-at-a-Time hash
*/
guint
ipv6_oat_hash(gconstpointer key)
{
	int len = 16;
	const unsigned char *p = (const unsigned char *)key;
	guint h = 0;
	int i;

	for (i = 0; i < len; i++) {
		h += p[i];
		h += (h << 10);
		h ^= (h >> 6);
	}

	h += (h << 3);
	h ^= (h >> 11);
	h += (h << 15);

	return h;
}
gboolean
ipv6_equal(gconstpointer v1, gconstpointer v2)
{

	if (memcmp(v1, v2, sizeof(ws_in6_addr)) == 0) {
		return TRUE;
	}

	return FALSE;
}
static wmem_list_t *manually_resolved_ipv4_list = NULL;
wmem_list_t *
wmem_list_new(wmem_allocator_t *allocator)
{
	wmem_list_t *list;

	list = wmem_new(allocator, wmem_list_t);

	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
	list->allocator = allocator;

	return list;
}
static wmem_list_t *manually_resolved_ipv6_list = NULL;

/**
* @brief Flags to control name resolution.
*/
typedef struct _e_addr_resolve {
	gboolean mac_name;                          /**< Whether to resolve Ethernet MAC to manufacturer names */
	gboolean network_name;                      /**< Whether to resolve IPv4, IPv6, and IPX addresses into host names */
	gboolean transport_name;                    /**< Whether to resolve TCP/UDP/DCCP/SCTP ports into service names */
	gboolean dns_pkt_addr_resolution;           /**< Whether to resolve addresses using captured DNS packets */
	gboolean use_external_net_name_resolver;    /**< Whether to system's configured DNS server to resolve names */
	gboolean load_hosts_file_from_profile_only; /**< Whether to only load the hosts in the current profile, not hosts files */
	gboolean vlan_name;                         /**< Whether to resolve VLAN IDs to names */
	gboolean ss7pc_name;                        /**< Whether to resolve SS7 Point Codes to names */
} e_addr_resolve;
e_addr_resolve gbl_resolv_flags = {
	TRUE,   /* mac_name */
	FALSE,  /* network_name */
	FALSE,  /* transport_name */
	TRUE,   /* dns_pkt_addr_resolution */
	TRUE,   /* use_external_net_name_resolver */
	FALSE,  /* load_hosts_file_from_profile_only */
	FALSE,  /* vlan_name */
	FALSE   /* ss7 point code names */
};
/*
* TRUE if we're running from the build directory and we aren't running
* with special privileges.
*/
static gboolean running_in_build_directory_flag = FALSE;
/*
* Directory from which the executable came.
*/
static char *progfile_dir;
/*
* Get the directory in which the program resides.
*/
const char *
get_progfile_dir(void)
{
	return progfile_dir;
}
/*
* Get the directory in which the global configuration and data files are
* stored.
*
* On Windows, we use the directory in which the executable for this
* process resides.
*
* On UN*X, we use the DATAFILE_DIR value supplied by the configure
* script, unless we think we're being run from the build directory,
* in which case we use the directory in which the executable for this
* process resides.
*
* XXX - if we ever make libwireshark a real library, used by multiple
* applications (more than just TShark and versions of Wireshark with
* various UIs), should the configuration files belong to the library
* (and be shared by all those applications) or to the applications?
*
* If they belong to the library, that could be done on UNIX by the
* configure script, but it's trickier on Windows, as you can't just
* use the pathname of the executable.
*
* If they belong to the application, that could be done on Windows
* by using the pathname of the executable, but we'd have to have it
* passed in as an argument, in some call, on UNIX.
*
* Note that some of those configuration files might be used by code in
* libwireshark, some of them might be used by dissectors (would they
* belong to libwireshark, the application, or a separate library?),
* and some of them might be used by other code (the Wireshark preferences
* file includes resolver preferences that control the behavior of code
* in libwireshark, dissector preferences, and UI preferences, for
* example).
*/
const char *
get_datafile_dir(void)
{
	static const char *datafile_dir = NULL;

	if (datafile_dir != NULL)
		return datafile_dir;

#ifdef _WIN32
	/*
	* Do we have the pathname of the program?  If so, assume we're
	* running an installed version of the program.  If we fail,
	* we don't change "datafile_dir", and thus end up using the
	* default.
	*
	* XXX - does NSIS put the installation directory into
	* "\HKEY_LOCAL_MACHINE\SOFTWARE\Wireshark\InstallDir"?
	* If so, perhaps we should read that from the registry,
	* instead.
	*/
	if (progfile_dir != NULL) {
		/*
		* Yes, we do; use that.
		*/
		datafile_dir = progfile_dir;
	}
	else {
		/*
		* No, we don't.
		* Fall back on the default installation directory.
		*/
		datafile_dir = "C:\\Program Files\\Wireshark\\";
	}
#else

	if (running_in_build_directory_flag) {
		/*
		* We're (probably) being run from the build directory and
		* weren't started with special privileges.
		*
		* (running_in_build_directory_flag is never set to TRUE
		* if we're started with special privileges, so we need
		* only check it; we don't need to call started_with_special_privs().)
		*
		* Data files (console.lua, radius/, etc.) are copied to the build
		* directory during the build.
		*/
		datafile_dir = BUILD_TIME_DATAFILE_DIR;
		return datafile_dir;
	}
	else {
		if (g_getenv("WIRESHARK_DATA_DIR") && !started_with_special_privs()) {
			/*
			* The user specified a different directory for data files
			* and we aren't running with special privileges.
			* XXX - We might be able to dispense with the priv check
			*/
			datafile_dir = g_strdup(g_getenv("WIRESHARK_DATA_DIR"));
		}


#endif
	return datafile_dir;
}
/*
* Construct the path name of a global configuration file, given the
* file name.
*
* The returned file name was g_malloc()'d so it must be g_free()d when the
* caller is done with it.
*/
char *
get_datafile_path(const char *filename)
{
	if (running_in_build_directory_flag &&
		(!strcmp(filename, "AUTHORS-SHORT") ||
			!strcmp(filename, "hosts"))) {
		/* We're running in the build directory and the requested file is a
		* generated (or a test) file.  Return the file name in the build
		* directory (not in the source/data directory).
		* (Oh the things we do to keep the source directory pristine...)
		*/
		return g_build_filename(get_progfile_dir(), filename, (char *)NULL);
	}
	else {
		return g_build_filename(get_datafile_dir(), filename, (char *)NULL);
	}
}
#define ENAME_HOSTS     "hosts"
#define ws_fopen		fopen
int	getc_unlocked(FILE *);
#define ws_getc_unlocked getc_unlocked
void *
wmem_realloc(wmem_allocator_t *allocator, void *ptr, const size_t size)
{
	if (allocator == NULL) {
		return g_realloc(ptr, size);
	}

	if (ptr == NULL) {
		return wmem_alloc(allocator, size);
	}

	if (size == 0) {
		wmem_free(allocator, ptr);
		return NULL;
	}

	g_assert(allocator->in_scope);

	return allocator->wrealloc(allocator->private_data, ptr, size);
}
/*
*  Miscellaneous functions
*/
static int
fgetline(char **buf, int *size, FILE *fp)
{
	int len;
	int c;

	if (fp == NULL || buf == NULL)
		return -1;

	if (*buf == NULL) {
		if (*size == 0)
			*size = BUFSIZ;

		*buf = (char *)wmem_alloc(wmem_epan_scope(), *size);
	}

	g_assert(*buf);
	g_assert(*size > 0);

	if (feof(fp))
		return -1;

	len = 0;
	while ((c = ws_getc_unlocked(fp)) != EOF && c != '\r' && c != '\n') {
		if (len + 1 >= *size) {
			*buf = (char *)wmem_realloc(wmem_epan_scope(), *buf, *size += BUFSIZ);
		}
		(*buf)[len++] = c;
	}

	if (len == 0 && c == EOF)
		return -1;

	(*buf)[len] = '\0';

	return len;

} /* fgetline */
  /*
  * We assume and require an inet_pton/inet_ntop that supports AF_INET
  * and AF_INET6.
  */
//#include <arpa/inet.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
static inline gboolean
_inet_pton(int af, const gchar *src, gpointer dst)
{
	gint ret = inet_pton(af, src, dst);
	if (G_UNLIKELY(ret < 0)) {
		/* EAFNOSUPPORT */
		if (af == AF_INET) {
			memset(dst, 0, sizeof(struct in_addr));
			g_critical("ws_inet_pton4: EAFNOSUPPORT");
		}
		else if (af == AF_INET6) {
			memset(dst, 0, sizeof(struct in6_addr));
			g_critical("ws_inet_pton6: EAFNOSUPPORT");
		}
		else {
			g_assert(0);
		}
		errno = EAFNOSUPPORT;
	}
	return ret == 1;
}

static inline const gchar *
_inet_ntop(int af, gconstpointer src, gchar *dst, guint dst_size)
{
	const gchar *ret = inet_ntop(af, src, dst, dst_size);
	if (G_UNLIKELY(ret == NULL)) {
		int saved_errno = errno;
		//gchar *errmsg;
		//errmsg = ;
		switch (errno) {
		case EAFNOSUPPORT:
			g_critical("ws_inet_ntop: EAFNOSUPPORT");
			return "<<EAFNOSUPPORT>>";
			break;
		case ENOSPC:
			return "<<ENOSPC>>";
			break;
		default:
			return "<<ERROR>>";
			errno = saved_errno;
			break;
		}
		/* set result to something that can't be confused with a valid conversion */
		//g_strlcpy(dst, errmsg, dst_size);
		/* set errno for caller */
		
	}
	return dst;
}
const gchar *
ws_inet_ntop4(gconstpointer src, gchar *dst, guint dst_size)
{
	return _inet_ntop(AF_INET, src, dst, dst_size);
}

gboolean
ws_inet_pton4(const gchar *src, guint32 *dst)
{
	return _inet_pton(AF_INET, src, dst);
}

const gchar *
ws_inet_ntop6(gconstpointer src, gchar *dst, guint dst_size)
{
	return _inet_ntop(AF_INET6, src, dst, dst_size);
}

gboolean
ws_inet_pton6(const gchar *src, ws_in6_addr *dst)
{
	return _inet_pton(AF_INET6, src, dst);
}
#define WS_INET_ADDRSTRLEN      16
#define WS_INET6_ADDRSTRLEN     46
typedef struct hashipv4 {
	guint             addr;
	guint8            flags;          /* B0 dummy_entry, B1 resolve, B2 If the address is used in the trace */
	gchar             ip[WS_INET_ADDRSTRLEN];
	gchar             name[MAXNAMELEN];
} hashipv4_t;
static guint32 x; /* Used for universal integer hashing (see the HASH macro) */

#define HASH(MAP, KEY) \
    ((guint32)(((MAP)->hash_func(KEY) * x) >> (32 - (MAP)->capacity)))

void *
wmem_map_lookup(wmem_map_t *map, const void *key)
{
	wmem_map_item_t *item;

	/* Make sure we have a table */
	if (map->table == NULL) {
		return NULL;
	}

	/* find correct slot */
	item = map->table[HASH(map, key)];

	/* scan list of items in this slot for the correct value */
	while (item) {
		if (map->eql_func(key, item->key)) {
			return item->value;
		}
		item = item->next;
	}

	return NULL;
}

#define BUF_TOO_SMALL_ERR "[Buffer too small]"
static const char fast_strings[][4] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "10", "11", "12", "13", "14", "15",
	"16", "17", "18", "19", "20", "21", "22", "23",
	"24", "25", "26", "27", "28", "29", "30", "31",
	"32", "33", "34", "35", "36", "37", "38", "39",
	"40", "41", "42", "43", "44", "45", "46", "47",
	"48", "49", "50", "51", "52", "53", "54", "55",
	"56", "57", "58", "59", "60", "61", "62", "63",
	"64", "65", "66", "67", "68", "69", "70", "71",
	"72", "73", "74", "75", "76", "77", "78", "79",
	"80", "81", "82", "83", "84", "85", "86", "87",
	"88", "89", "90", "91", "92", "93", "94", "95",
	"96", "97", "98", "99", "100", "101", "102", "103",
	"104", "105", "106", "107", "108", "109", "110", "111",
	"112", "113", "114", "115", "116", "117", "118", "119",
	"120", "121", "122", "123", "124", "125", "126", "127",
	"128", "129", "130", "131", "132", "133", "134", "135",
	"136", "137", "138", "139", "140", "141", "142", "143",
	"144", "145", "146", "147", "148", "149", "150", "151",
	"152", "153", "154", "155", "156", "157", "158", "159",
	"160", "161", "162", "163", "164", "165", "166", "167",
	"168", "169", "170", "171", "172", "173", "174", "175",
	"176", "177", "178", "179", "180", "181", "182", "183",
	"184", "185", "186", "187", "188", "189", "190", "191",
	"192", "193", "194", "195", "196", "197", "198", "199",
	"200", "201", "202", "203", "204", "205", "206", "207",
	"208", "209", "210", "211", "212", "213", "214", "215",
	"216", "217", "218", "219", "220", "221", "222", "223",
	"224", "225", "226", "227", "228", "229", "230", "231",
	"232", "233", "234", "235", "236", "237", "238", "239",
	"240", "241", "242", "243", "244", "245", "246", "247",
	"248", "249", "250", "251", "252", "253", "254", "255"
};
void
ip_to_str_buf(const guint8 *ad, gchar *buf, const int buf_len)
{
	register gchar const *p;
	register gchar *b = buf;

	if (buf_len < WS_INET_ADDRSTRLEN) {
		g_strlcpy(buf, BUF_TOO_SMALL_ERR, buf_len);  /* Let the unexpected value alert user */
		return;
	}

	p = fast_strings[*ad++];
	do {
		*b++ = *p;
		p++;
	} while (*p);
	*b++ = '.';

	p = fast_strings[*ad++];
	do {
		*b++ = *p;
		p++;
	} while (*p);
	*b++ = '.';

	p = fast_strings[*ad++];
	do {
		*b++ = *p;
		p++;
	} while (*p);
	*b++ = '.';

	p = fast_strings[*ad];
	do {
		*b++ = *p;
		p++;
	} while (*p);
	*b = 0;
}

/* --------------- */
static hashipv4_t *
new_ipv4(const guint addr)
{
	hashipv4_t *tp = wmem_new(wmem_epan_scope(), hashipv4_t);
	tp->addr = addr;
	tp->flags = 0;
	tp->name[0] = '\0';
	ip_to_str_buf((const guint8 *)&addr, tp->ip, sizeof(tp->ip));
	return tp;
}

#define WMEM_MAP_DEFAULT_CAPACITY 5
#define wmem_safe_mult(A, B) \
    ((((A) <= 0) || ((B) <= 0) || ((gsize)(A) > (G_MAXSSIZE / (gsize)(B)))) ? 0 : ((A) * (B)))

void *
wmem_alloc0(wmem_allocator_t *allocator, const size_t size)
G_GNUC_MALLOC;

#define wmem_alloc0_array(allocator, type, num) \
    ((type*)wmem_alloc0((allocator), wmem_safe_mult(sizeof(type), (num))))
#define CAPACITY(MAP) (((size_t)1) << (MAP)->capacity)

static void
wmem_map_init_table(wmem_map_t *map)
{
	map->count = 0;
	map->capacity = WMEM_MAP_DEFAULT_CAPACITY;
	map->table = wmem_alloc0_array(map->allocator, wmem_map_item_t*, CAPACITY(map));
}

static inline void
wmem_map_grow(wmem_map_t *map)
{
	wmem_map_item_t **old_table, *cur, *nxt;
	size_t            old_cap, i;
	guint             slot;

	/* store the old table and capacity */
	old_table = map->table;
	old_cap = CAPACITY(map);

	/* double the size (capacity is base-2 logarithm, so this just means
	* increment it) and allocate new table */
	map->capacity++;
	map->table = wmem_alloc0_array(map->allocator, wmem_map_item_t*, CAPACITY(map));

	/* copy all the elements over from the old table */
	for (i = 0; i<old_cap; i++) {
		cur = old_table[i];
		while (cur) {
			nxt = cur->next;
			slot = HASH(map, cur->key);
			cur->next = map->table[slot];
			map->table[slot] = cur;
			cur = nxt;
		}
	}

	/* free the old table */
	wmem_free(map->allocator, old_table);
}


void *
wmem_map_insert(wmem_map_t *map, const void *key, void *value)
{
	wmem_map_item_t **item;
	void *old_val;

	/* Make sure we have a table */
	if (map->table == NULL) {
		wmem_map_init_table(map);
	}

	/* get a pointer to the slot */
	item = &(map->table[HASH(map, key)]);

	/* check existing items in that slot */
	while (*item) {
		if (map->eql_func(key, (*item)->key)) {
			/* replace and return old value for this key */
			old_val = (*item)->value;
			(*item)->value = value;
			return old_val;
		}
		item = &((*item)->next);
	}

	/* insert new item */
	(*item) = wmem_new(map->allocator, wmem_map_item_t);

	(*item)->key = key;
	(*item)->value = value;
	(*item)->next = NULL;

	map->count++;

	/* increase size if we are over-full */
	if (map->count >= CAPACITY(map)) {
		wmem_map_grow(map);
	}

	/* no previous entry, return NULL */
	return NULL;
}

#define TRIED_RESOLVE_ADDRESS    (1U<<0)  /* XXX - what does this bit *really* mean? */
#define NAME_RESOLVED            (1U<<1)  /* the name field contains a host name, not a printable address */
#define RESOLVED_ADDRESS_USED    (1U<<2)  /* a get_hostname* call returned the host name */


/* -------------------------- */
void
add_ipv4_name(const guint addr, const gchar *name)
{
	hashipv4_t *tp;

	/*
	* Don't add zero-length names; apparently, some resolvers will return
	* them if they get them from DNS.
	*/
	if (!name || name[0] == '\0')
		return;

	tp = (hashipv4_t *)wmem_map_lookup(ipv4_hash_table, GUINT_TO_POINTER(addr));
	if (!tp) {
		tp = new_ipv4(addr);
		wmem_map_insert(ipv4_hash_table, GUINT_TO_POINTER(addr), tp);
	}

	if (g_ascii_strcasecmp(tp->name, name)) {
		g_strlcpy(tp->name, name, MAXNAMELEN);
		new_resolved_objects = TRUE;
	}
	tp->flags |= TRIED_RESOLVE_ADDRESS | NAME_RESOLVED;
} /* add_ipv4_name */

typedef struct hashipv6 {
	guint8            addr[16];
	guint8            flags;          /* B0 dummy_entry, B1 resolve, B2 If the address is used in the trace */
	gchar             ip6[WS_INET6_ADDRSTRLEN];
	gchar             name[MAXNAMELEN];
} hashipv6_t;

int
ip6_to_str_buf(const ws_in6_addr *addr, gchar *buf, int buf_size)
{
	gchar addr_buf[WS_INET6_ADDRSTRLEN];
	int len;

	/* slightly more efficient than ip6_to_str_buf_with_pfx(addr, buf, buf_size, NULL) */
	len = (int)g_strlcpy(buf, ws_inet_ntop6(addr, addr_buf, sizeof(addr_buf)), buf_size);     /* this returns len = strlen(addr_buf) */

	if (len > buf_size - 1) { /* size minus nul terminator */
		len = (int)g_strlcpy(buf, BUF_TOO_SMALL_ERR, buf_size);  /* Let the unexpected value alert user */
	}
	return len;
}
/* --------------- */
static hashipv6_t *
new_ipv6(const ws_in6_addr *addr)
{
	hashipv6_t *tp = wmem_new(wmem_epan_scope(), hashipv6_t);
	memcpy(tp->addr, addr->bytes, sizeof tp->addr);
	tp->flags = 0;
	tp->name[0] = '\0';
	ip6_to_str_buf(addr, tp->ip6, sizeof(tp->ip6));
	return tp;
}
  /* -------------------------- */
void
add_ipv6_name(const ws_in6_addr *addrp, const gchar *name)
{
	hashipv6_t *tp;

	/*
	* Don't add zero-length names; apparently, some resolvers will return
	* them if they get them from DNS.
	*/
	if (!name || name[0] == '\0')
		return;

	tp = (hashipv6_t *)wmem_map_lookup(ipv6_hash_table, addrp);
	if (!tp) {
		ws_in6_addr *addr_key;

		addr_key = wmem_new(wmem_epan_scope(), ws_in6_addr);
		tp = new_ipv6(addrp);
		memcpy(addr_key, addrp, 16);
		wmem_map_insert(ipv6_hash_table, addr_key, tp);
	}

	if (g_ascii_strcasecmp(tp->name, name)) {
		g_strlcpy(tp->name, name, MAXNAMELEN);
		new_resolved_objects = TRUE;
	}
	tp->flags |= TRIED_RESOLVE_ADDRESS | NAME_RESOLVED;
} /* add_ipv6_name */

static gboolean
read_hosts_file(const char *hostspath, gboolean store_entries)
{
	FILE *hf;
	char *line = NULL;
	int size = 0;
	gchar *cp;
	union {
		guint32 ip4_addr;
		ws_in6_addr ip6_addr;
	} host_addr;
	gboolean is_ipv6, entry_found = FALSE;

	/*
	*  See the hosts(4) or hosts(5) man page for hosts file format
	*  (not available on all systems).
	*/
	if ((hf = ws_fopen(hostspath, "r")) == NULL)
		return FALSE;

	while (fgetline(&line, &size, hf) >= 0) {
		if ((cp = strchr(line, '#')))
			*cp = '\0';

		if ((cp = strtok(line, " \t")) == NULL)
			continue; /* no tokens in the line */

		if (ws_inet_pton6(cp, &host_addr.ip6_addr)) {
			/* Valid IPv6 */
			is_ipv6 = TRUE;
		}
		else if (ws_inet_pton4(cp, &host_addr.ip4_addr)) {
			/* Valid IPv4 */
			is_ipv6 = FALSE;
		}
		else {
			continue;
		}

		if ((cp = strtok(NULL, " \t")) == NULL)
			continue; /* no host name */

		entry_found = TRUE;
		if (store_entries) {
			if (is_ipv6) {
				add_ipv6_name(&host_addr.ip6_addr, cp);
			}
			else {
				add_ipv4_name(host_addr.ip4_addr, cp);
			}
		}
	}
	wmem_free(wmem_epan_scope(), line);

	fclose(hf);
	return entry_found ? TRUE : FALSE;
} /* read_hosts_file */
static void(*report_open_failure_func)(const char *, int, gboolean);

  /*
  * Report an error when trying to open or create a file.
  * "err" is assumed to be an error code from Wiretap; positive values are
  * UNIX-style errnos, so this can be used for open failures not from
  * Wiretap as long as the failure code is just an errno.
  */
void
report_open_failure(const char *filename, int err,
	gboolean for_writing)
{
	(*report_open_failure_func)(filename, err, for_writing);
}


static gboolean do_store_persconffiles = FALSE;
static GHashTable *profile_files = NULL;
char *persconffile_dir = NULL;

/*
* Default profile name.
*/
#define DEFAULT_PROFILE      "Default"
/*
* Get the directory in which personal configuration files reside.
*
* On Windows, it's "Wireshark", under %APPDATA% or, if %APPDATA% isn't set,
* it's "%USERPROFILE%\Application Data" (which is what %APPDATA% normally
* is on Windows 2000).
*
* On UNIX-compatible systems, we first look in XDG_CONFIG_HOME/wireshark
* and, if that doesn't exist, ~/.wireshark, for backwards compatibility.
* If neither exists, we use XDG_CONFIG_HOME/wireshark, so that the directory
* is initially created as XDG_CONFIG_HOME/wireshark.  We use that regardless
* of whether the user is running under an XDG desktop or not, so that
* if the user's home directory is on a server and shared between
* different desktop environments on different machines, they can all
* share the same configuration file directory.
*
* XXX - what about stuff that shouldn't be shared between machines,
* such as plugins in the form of shared loadable images?
*/
static const char *
get_persconffile_dir_no_profile(void)
{
#ifdef _WIN32
	const char *env;
#else
	char *xdg_path, *path;
	struct passwd *pwd;
	const char *homedir;
#endif

	/* Return the cached value, if available */
	if (persconffile_dir != NULL)
		return persconffile_dir;

#ifdef _WIN32
	/*
	* See if the user has selected an alternate environment.
	*/
	env = g_getenv("WIRESHARK_APPDATA");
	if (env != NULL) {
		persconffile_dir = g_strdup(env);
		return persconffile_dir;
	}

	/*
	* Use %APPDATA% or %USERPROFILE%, so that configuration
	* files are stored in the user profile, rather than in
	* the home directory.  The Windows convention is to store
	* configuration information in the user profile, and doing
	* so means you can use Wireshark even if the home directory
	* is an inaccessible network drive.
	*/
	env = g_getenv("APPDATA");
	if (env != NULL) {
		/*
		* Concatenate %APPDATA% with "\Wireshark".
		*/
		persconffile_dir = g_build_filename(env, "Wireshark", NULL);
		return persconffile_dir;
	}

	/*
	* OK, %APPDATA% wasn't set, so use %USERPROFILE%\Application Data.
	*/
	env = g_getenv("USERPROFILE");
	if (env != NULL) {
		persconffile_dir = g_build_filename(env, "Application Data", "Wireshark", NULL);
		return persconffile_dir;
	}

	/*
	* Give up and use "C:".
	*/
	persconffile_dir = g_build_filename("C:", "Wireshark", NULL);
	return persconffile_dir;
#else
	/*
	* Check if XDG_CONFIG_HOME/wireshark exists and is a directory.
	*/
	xdg_path = g_build_filename(g_get_user_config_dir(), "wireshark", NULL);
	if (g_file_test(xdg_path, G_FILE_TEST_IS_DIR)) {
		persconffile_dir = xdg_path;
		return persconffile_dir;
	}

	/*
	* It doesn't exist, or it does but isn't a directory, so try
	* ~/.wireshark.
	*
	* If $HOME is set, use that for ~.
	*
	* (Note: before GLib 2.36, g_get_home_dir() didn't look at $HOME,
	* but we always want to do so, so we don't use g_get_home_dir().)
	*/
	homedir = g_getenv("HOME");
	if (homedir == NULL) {
		/*
		* It's not set.
		*
		* Get their home directory from the password file.
		* If we can't even find a password file entry for them,
		* use "/tmp".
		*/
		pwd = getpwuid(getuid());
		if (pwd != NULL) {
			homedir = pwd->pw_dir;
		}
		else {
			homedir = "/tmp";
		}
	}
	path = g_build_filename(homedir, ".wireshark", NULL);
	if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
		g_free(xdg_path);
		persconffile_dir = path;
		return persconffile_dir;
	}

	/*
	* Neither are directories that exist; use the XDG path, so we'll
	* create that as necessary.
	*/
	g_free(path);
	persconffile_dir = xdg_path;
	return persconffile_dir;
#endif
}
#define PROFILES_DIR    "profiles"

char *
get_profiles_dir(void)
{
	return g_strdup_printf("%s%s%s", get_persconffile_dir_no_profile(),
		G_DIR_SEPARATOR_S, PROFILES_DIR);
}
static char *
get_persconffile_dir(const gchar *profilename)
{
	char *persconffile_profile_dir = NULL, *profile_dir;

	if (profilename && strlen(profilename) > 0 &&
		strcmp(profilename, DEFAULT_PROFILE) != 0) {
		profile_dir = get_profiles_dir();
		persconffile_profile_dir = g_strdup_printf("%s%s%s", profile_dir,
			G_DIR_SEPARATOR_S, profilename);
		g_free(profile_dir);
	}
	else {
		persconffile_profile_dir = g_strdup(get_persconffile_dir_no_profile());
	}

	return persconffile_profile_dir;
}
char *persconfprofile = NULL;
/*
* Construct the path name of a personal configuration file, given the
* file name.
*
* On Win32, if "for_writing" is FALSE, we check whether the file exists
* and, if not, construct a path name relative to the ".wireshark"
* subdirectory of the user's home directory, and check whether that
* exists; if it does, we return that, so that configuration files
* from earlier versions can be read.
*
* The returned file name was g_malloc()'d so it must be g_free()d when the
* caller is done with it.
*/
char *
get_persconffile_path(const char *filename, gboolean from_profile)
{
	char *path, *dir = NULL;

	if (do_store_persconffiles && from_profile && !g_hash_table_lookup(profile_files, filename)) {
		/* Store filenames so we know which filenames belongs to a configuration profile */
		g_hash_table_insert(profile_files, g_strdup(filename), g_strdup(filename));
	}

	if (from_profile) {
		dir = get_persconffile_dir(persconfprofile);
	}
	else {
		dir = get_persconffile_dir(NULL);
	}
	path = g_build_filename(dir, filename, NULL);

	g_free(dir);
	return path;
}
static GPtrArray* extra_hosts_files = NULL;
guint32
ip_get_subnet_mask(const guint32 mask_length)
{
	static const guint32 masks[33] = {
		0x00000000,
		0x80000000, 0xc0000000, 0xe0000000, 0xf0000000,
		0xf8000000, 0xfc000000, 0xfe000000, 0xff000000,
		0xff800000, 0xffc00000, 0xffe00000, 0xfff00000,
		0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000,
		0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
		0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
		0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0,
		0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff,
	};

	g_assert(mask_length <= 32);

	return masks[mask_length];
}
#define ENAME_SUBNETS   "subnets"
gboolean
str_to_ip(const char *str, void *dst)
{
	return ws_inet_pton4(str, (guint32 *)dst);
}

gboolean
str_to_ip6(const char *str, void *dst)
{
	return ws_inet_pton6(str, (ws_in6_addr *)dst);
}
/*
* NOTE: G_HAVE_GNUC_VISIBILITY is defined only if all of
*
*    __attribute__ ((visibility ("hidden")))
*
*    __attribute__ ((visibility ("internal")))
*
*    __attribute__ ((visibility ("protected")))
*
*    __attribute__ ((visibility ("default")))
*
* are supported, and at least some versions of GCC from Apple support
* "default" and "hidden" but not "internal" or "protected", so it
* shouldn't be used to determine whether "hidden" or "default" is
* supported.
*
* This also means that we shouldn't use G_GNUC_INTERNAL instead of
* WS_DLL_LOCAL, as GLib uses G_HAVE_GNUC_VISIBILITY to determine
* whether to use __attribute__ ((visibility ("hidden"))) for
* G_GNUC_INTERNAL, and that will not use it even with compilers
* that support it.
*/

/* Originally copied from GCC Wiki at http://gcc.gnu.org/wiki/Visibility */
#if defined _WIN32 || defined __CYGWIN__
/* Compiling for Windows, so we use the Windows DLL declarations. */
#ifdef WS_BUILD_DLL
/*
* Building a DLL; for all definitions, we want dllexport, and
* (presumably so source from DLL and source from a program using the
* DLL can both include a header that declares APIs and exported data
* for the DLL), for declarations, either dllexport or dllimport will
* work (they mean the same thing for a declaration when building a DLL).
*/
#ifdef __GNUC__
/* GCC */
#define WS_DLL_PUBLIC_DEF __attribute__ ((dllexport))
#else /* ! __GNUC__ */
/*
* Presumably MSVC.
* Note: actually gcc seems to also support this syntax.
*/
#define WS_DLL_PUBLIC_DEF __declspec(dllexport)
#endif /* __GNUC__ */
#else /* WS_BUILD_DLL */
/*
* Building a program; we should only see declarations, not definitions,
* with WS_DLL_PUBLIC, and they all represent APIs or data imported
* from a DLL, so use dllimport.
*
* For functions, export shouldn't be necessary; for data, it might
* be necessary, e.g. if what's declared is an array whose size is
* not given in the declaration.
*/
#ifdef __GNUC__
/* GCC */
#define WS_DLL_PUBLIC_DEF __attribute__ ((dllimport))
#elif ! (defined ENABLE_STATIC) /* ! __GNUC__ */
/*
* Presumably MSVC, and we're not building all-static.
* Note: actually gcc seems to also support this syntax.
*/
#define WS_DLL_PUBLIC_DEF __declspec(dllimport)
#else /* ! __GNUC__  && ENABLE_STATIC */
/*
* Presumably MSVC, and we're building all-static, so we're
* not building any DLLs.
*/
#define WS_DLL_PUBLIC_DEF
#endif /* __GNUC__ */
#endif /* WS_BUILD_DLL */

/*
* Symbols in a DLL are *not* exported unless they're specifically
* flagged as exported, so, for a non-static but non-exported
* symbol, we don't have to do anything.
*/
#define WS_DLL_LOCAL
#else /* defined _WIN32 || defined __CYGWIN__ */
/*
* Compiling for UN*X, where the dllimport and dllexport stuff
* is neither necessary nor supported; just specify the
* visibility if we have a compiler that supports doing so.
*/
#if WS_IS_AT_LEAST_GNUC_VERSION(3,4) \
      || WS_IS_AT_LEAST_XL_C_VERSION(12,0)
/*
* GCC 3.4 or later, or some compiler asserting compatibility with
* GCC 3.4 or later, or XL C 13.0 or later, so we have
* __attribute__((visibility()).
*/

/*
* Symbols exported from libraries.
*/
#define WS_DLL_PUBLIC_DEF __attribute__ ((visibility ("default")))

/*
* Non-static symbols *not* exported from libraries.
*/
#define WS_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#elif WS_IS_AT_LEAST_SUNC_VERSION(5,5)
/*
* Sun C 5.5 or later, so we have __global and __hidden.
* (Sun C 5.9 and later also have __attribute__((visibility()),
* but there's no reason to prefer it with Sun C.)
*/

/*
* Symbols exported from libraries.
*/
#define WS_DLL_PUBLIC_DEF __global

/*
* Non-static symbols *not* exported from libraries.
*/
#define WS_DLL_LOCAL __hidden
#else
/*
* We have neither a way to make stuff not explicitly marked as
* visible invisible outside a library nor a way to make stuff
* explicitly marked as local invisible outside the library.
*/

/*
* Symbols exported from libraries.
*/
#define WS_DLL_PUBLIC_DEF

/*
* Non-static symbols *not* exported from libraries.
*/
#define WS_DLL_LOCAL
#endif
#endif
/*
* You *must* use this for exported data *declarations*; if you use
* WS_DLL_PUBLIC_DEF, some compilers, such as MSVC++, will complain
* about array definitions with no size.
*
* You must *not* use this for exported data *definitions*, as that
* will, for some compilers, cause warnings about items being initialized
* and declared extern.
*
* Either can be used for exported *function* declarations and definitions.
*/
#define WS_DLL_PUBLIC  WS_DLL_PUBLIC_DEF extern
/* Read in a list of subnet definition - name pairs.
* <line> = <comment> | <entry> | <whitespace>
* <comment> = <whitespace>#<any>
* <entry> = <subnet_definition> <whitespace> <subnet_name> [<comment>|<whitespace><any>]
* <subnet_definition> = <ipv4_address> / <subnet_mask_length>
* <ipv4_address> is a full address; it will be masked to get the subnet-ID.
* <subnet_mask_length> is a decimal 1-31
* <subnet_name> is a string containing no whitespace.
* <whitespace> = (space | tab)+
* Any malformed entries are ignored.
* Any trailing data after the subnet_name is ignored.
*
* XXX Support IPv6
*/


WS_DLL_PUBLIC gboolean ws_strtou8(const gchar* str, const gchar** endptr, guint8*  cint);
#define HASH_IPV4_ADDRESS(addr) (g_htonl(addr) & (HASHHOSTSIZE - 1))

/* Add a subnet-definition - name pair to the set.
* The definition is taken by masking the address passed in with the mask of the
* given length.
*/
static void
subnet_entry_set(guint32 subnet_addr, const guint8 mask_length, const gchar* name)
{
	subnet_length_entry_t* entry;
	sub_net_hashipv4_t * tp;
	gsize hash_idx;

	g_assert(mask_length > 0 && mask_length <= 32);

	entry = &subnet_length_entries[mask_length - 1];

	subnet_addr &= entry->mask;

	hash_idx = HASH_IPV4_ADDRESS(subnet_addr);

	if (NULL == entry->subnet_addresses) {
		entry->subnet_addresses = (sub_net_hashipv4_t**)wmem_alloc0(wmem_epan_scope(), sizeof(sub_net_hashipv4_t*) * HASHHOSTSIZE);
	}

	if (NULL != (tp = entry->subnet_addresses[hash_idx])) {
		sub_net_hashipv4_t * new_tp;

		while (tp->next) {
			if (tp->addr == subnet_addr) {
				return; /* XXX provide warning that an address was repeated? */
			}
			else {
				tp = tp->next;
			}
		}

		new_tp = wmem_new(wmem_epan_scope(), sub_net_hashipv4_t);
		tp->next = new_tp;
		tp = new_tp;
	}
	else {
		tp = entry->subnet_addresses[hash_idx] = wmem_new(wmem_epan_scope(), sub_net_hashipv4_t);
	}

	tp->next = NULL;
	tp->addr = subnet_addr;
	g_strlcpy(tp->name, name, MAXNAMELEN); /* This is longer than subnet names can actually be */
	have_subnet_entry = TRUE;
}
static gboolean
read_subnets_file(const char *subnetspath)
{
	FILE *hf;
	char *line = NULL;
	int size = 0;
	gchar *cp, *cp2;
	guint32 host_addr; /* IPv4 ONLY */
	guint8 mask_length;

	if ((hf = ws_fopen(subnetspath, "r")) == NULL)
		return FALSE;

	while (fgetline(&line, &size, hf) >= 0) {
		if ((cp = strchr(line, '#')))
			*cp = '\0';

		if ((cp = strtok(line, " \t")) == NULL)
			continue; /* no tokens in the line */


					  /* Expected format is <IP4 address>/<subnet length> */
		cp2 = strchr(cp, '/');
		if (NULL == cp2) {
			/* No length */
			continue;
		}
		*cp2 = '\0'; /* Cut token */
		++cp2;

		/* Check if this is a valid IPv4 address */
		if (!str_to_ip(cp, &host_addr)) {
			continue; /* no */
		}

		if (!ws_strtou8(cp2, NULL, &mask_length) || mask_length == 0 || mask_length > 32) {
			continue; /* invalid mask length */
		}

		if ((cp = strtok(NULL, " \t")) == NULL)
			continue; /* no subnet name */

		subnet_entry_set(host_addr, mask_length, cp);
	}
	wmem_free(wmem_epan_scope(), line);

	fclose(hf);
	return TRUE;
} /* read_subnets_file */
static void
subnet_name_lookup_init(void)
{
	gchar* subnetspath;
	guint32 i;

	for (i = 0; i < SUBNETLENGTHSIZE; ++i) {
		guint32 length = i + 1;

		subnet_length_entries[i].subnet_addresses = NULL;
		subnet_length_entries[i].mask_length = length;
		subnet_length_entries[i].mask = g_htonl(ip_get_subnet_mask(length));
	}

	/* Check profile directory before personal configuration */
	subnetspath = get_persconffile_path(ENAME_SUBNETS, TRUE);
	if (!read_subnets_file(subnetspath)) {
		if (errno != ENOENT) {
			report_open_failure(subnetspath, errno, FALSE);
		}

		g_free(subnetspath);
		subnetspath = get_persconffile_path(ENAME_SUBNETS, FALSE);
		if (!read_subnets_file(subnetspath) && errno != ENOENT) {
			report_open_failure(subnetspath, errno, FALSE);
		}
	}
	g_free(subnetspath);

	/*
	* Load the global subnets file, if we have one.
	*/
	subnetspath = get_datafile_path(ENAME_SUBNETS);
	if (!read_subnets_file(subnetspath) && errno != ENOENT) {
		report_open_failure(subnetspath, errno, FALSE);
	}
	g_free(subnetspath);
}
void
wmem_list_foreach(wmem_list_t *list, GFunc foreach_func, gpointer user_data)
{
	wmem_list_frame_t *cur;

	cur = list->head;

	while (cur) {
		foreach_func(cur->data, user_data);
		cur = cur->next;
	}
}
typedef struct _resolved_ipv4
{
	guint32          host_addr;
	char             name[MAXNAMELEN];
} resolved_ipv4_t;
typedef struct _resolved_ipv6
{
	ws_in6_addr  ip6_addr;
	char               name[MAXNAMELEN];
} resolved_ipv6_t;
static void
add_manually_resolved_ipv4(gpointer data, gpointer user_data _U_)
{
	resolved_ipv4_t *resolved_ipv4_entry = (resolved_ipv4_t *)data;

	add_ipv4_name(resolved_ipv4_entry->host_addr, resolved_ipv4_entry->name);
}
static void
add_manually_resolved_ipv6(gpointer data, gpointer user_data _U_)
{
	resolved_ipv6_t *resolved_ipv6_entry = (resolved_ipv6_t *)data;

	add_ipv6_name(&(resolved_ipv6_entry->ip6_addr), resolved_ipv6_entry->name);
}
static void
add_manually_resolved(void)
{
	if (manually_resolved_ipv4_list) {
		wmem_list_foreach(manually_resolved_ipv4_list, add_manually_resolved_ipv4, NULL);
	}

	if (manually_resolved_ipv6_list) {
		wmem_list_foreach(manually_resolved_ipv6_list, add_manually_resolved_ipv6, NULL);
	}
}
#define ENAME_SS7PCS    "ss7pcs"
WS_DLL_PUBLIC gboolean ws_strtou32(const gchar* str, const gchar** endptr, guint32* cint);

typedef struct ss7pc {
	guint32             id; /* 1st byte NI, 3 following bytes: Point Code */
	gchar               pc_addr[MAXNAMELEN];
	gchar               name[MAXNAMELEN];
} hashss7pc_t;
/* SS7 PC Name Resolution Portion */
static hashss7pc_t *
new_ss7pc(const guint8 ni, const guint32 pc)
{
	hashss7pc_t *tp = wmem_new(wmem_epan_scope(), hashss7pc_t);
	tp->id = (ni << 24) + (pc & 0xffffff);
	tp->pc_addr[0] = '\0';
	tp->name[0] = '\0';

	return tp;
}
static void
add_ss7pc_name(const guint8 ni, guint32 pc, const gchar *name)
{
	hashss7pc_t *tp;
	guint32 id;

	if (!name || name[0] == '\0')
		return;

	id = (ni << 24) + (pc & 0xffffff);
	tp = (hashss7pc_t *)wmem_map_lookup(ss7pc_hash_table, GUINT_TO_POINTER(id));
	if (!tp) {
		tp = new_ss7pc(ni, pc);
		wmem_map_insert(ss7pc_hash_table, GUINT_TO_POINTER(id), tp);
	}

	if (g_ascii_strcasecmp(tp->name, name)) {
		g_strlcpy(tp->name, name, MAXNAMELEN);
	}
}
static gboolean
read_ss7pcs_file(const char *ss7pcspath)
{
	FILE *hf;
	char *line = NULL;
	int size = 0;
	gchar *cp;
	guint8 ni;
	guint32 pc;
	gboolean entry_found = FALSE;

	/*
	*  File format is Network Indicator (decimal)<dash>Point Code (Decimal)<tab/space>Hostname
	*/
	if ((hf = ws_fopen(ss7pcspath, "r")) == NULL)
		return FALSE;

	while (fgetline(&line, &size, hf) >= 0) {
		if ((cp = strchr(line, '#')))
			*cp = '\0';

		if ((cp = strtok(line, "-")) == NULL)
			continue; /*no ni-pc separator*/
		if (!ws_strtou8(cp, NULL, &ni))
			continue;
		if (ni > 3)
			continue;

		if ((cp = strtok(NULL, " \t")) == NULL)
			continue; /* no tokens for pc and name */
		if (!ws_strtou32(cp, NULL, &pc))
			continue;
		if (pc >> 24 > 0)
			continue;

		if ((cp = strtok(NULL, " \t")) == NULL)
			continue; /* no host name */

		entry_found = TRUE;
		add_ss7pc_name(ni, pc, cp);
	}
	wmem_free(wmem_epan_scope(), line);

	fclose(hf);
	return entry_found ? TRUE : FALSE;
}
static void
ss7pc_name_lookup_init(void)
{
	char *ss7pcspath;

	g_assert(ss7pc_hash_table == NULL);

	ss7pc_hash_table = wmem_map_new(wmem_epan_scope(), g_direct_hash, g_direct_equal);

	/*
	* Load the user's ss7pcs file
	*/
	ss7pcspath = get_persconffile_path(ENAME_SS7PCS, TRUE);
	if (!read_ss7pcs_file(ss7pcspath) && errno != ENOENT) {
		report_open_failure(ss7pcspath, errno, FALSE);
	}
	g_free(ss7pcspath);
}
static void host_name_lookup_init(void)
{
	char *hostspath;
	guint i;

	g_assert(ipxnet_hash_table == NULL);
	ipxnet_hash_table = wmem_map_new(wmem_epan_scope(), g_int_hash, g_int_equal);

	g_assert(ipv4_hash_table == NULL);
	ipv4_hash_table = wmem_map_new(wmem_epan_scope(), g_direct_hash, g_direct_equal);

	g_assert(ipv6_hash_table == NULL);
	ipv6_hash_table = wmem_map_new(wmem_epan_scope(), ipv6_oat_hash, ipv6_equal);

#ifdef HAVE_C_ARES
	g_assert(async_dns_queue_head == NULL);
	async_dns_queue_head = wmem_list_new(wmem_epan_scope());
#endif

	if (manually_resolved_ipv4_list == NULL)
		manually_resolved_ipv4_list = wmem_list_new(wmem_epan_scope());

	if (manually_resolved_ipv6_list == NULL)
		manually_resolved_ipv6_list = wmem_list_new(wmem_epan_scope());

	/*
	* Load the global hosts file, if we have one.
	*/
	if (!gbl_resolv_flags.load_hosts_file_from_profile_only) {
		hostspath = get_datafile_path(ENAME_HOSTS);
		if (!read_hosts_file(hostspath, TRUE) && errno != ENOENT) {
			report_open_failure(hostspath, errno, FALSE);
		}
		g_free(hostspath);
	}
	/*
	* Load the user's hosts file no matter what, if they have one.
	*/
	hostspath = get_persconffile_path(ENAME_HOSTS, TRUE);
	if (!read_hosts_file(hostspath, TRUE) && errno != ENOENT) {
		report_open_failure(hostspath, errno, FALSE);
	}
	g_free(hostspath);
#ifdef HAVE_C_ARES
#ifdef CARES_HAVE_ARES_LIBRARY_INIT
	if (ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS) {
#endif
		if (ares_init(&ghba_chan) == ARES_SUCCESS && ares_init(&ghbn_chan) == ARES_SUCCESS) {
			async_dns_initialized = TRUE;
		}
#ifdef CARES_HAVE_ARES_LIBRARY_INIT
	}
#endif
#else
#endif /* HAVE_C_ARES */

	if (extra_hosts_files && !gbl_resolv_flags.load_hosts_file_from_profile_only) {
		for (i = 0; i < extra_hosts_files->len; i++) {
			read_hosts_file((const char *)g_ptr_array_index(extra_hosts_files, i), TRUE);
		}
	}

	subnet_name_lookup_init();

	add_manually_resolved();

	ss7pc_name_lookup_init();
}
void host_name_lookup_reset(void)
{
	host_name_lookup_cleanup();
	host_name_lookup_init();
}
/* Initialize all data structures used for dissection. */
void
wmem_enter_file_scope(void)
{
	g_assert(file_scope);
	g_assert(!file_scope->in_scope);

	file_scope->in_scope = TRUE;
}

static guint32 new_index;
/**
* Initialize some variables every time a file is loaded or re-loaded.
*/
void conversation_epan_reset(void)
{
	/*
	* Start the conversation indices over at 0.
	*/
	new_index = 0;
}
void
epan_conversation_init(void)
{
	conversation_epan_reset();
}
static GSList *init_routines = NULL;
/* key */
typedef struct stream_key {
	/* streams can be attached to circuits or conversations, and we note
	that here */
	gboolean is_circuit;
	union {
		const struct circuit *circuit;
		const struct conversation *conv;
	} circ;
	int p2p_dir;
} stream_key_t;
/* hash func */
static guint stream_hash_func(gconstpointer k)
{
	const stream_key_t *key = (const stream_key_t *)k;

	/* is_circuit is redundant to the circuit/conversation pointer */
	return (GPOINTER_TO_UINT(key->circ.circuit)) ^ key->p2p_dir;
}
/* compare func */
static gboolean stream_compare_func(gconstpointer a,
	gconstpointer b)
{
	const stream_key_t *key1 = (const stream_key_t *)a;
	const stream_key_t *key2 = (const stream_key_t *)b;
	if (key1->p2p_dir != key2->p2p_dir ||
		key1->is_circuit != key2->is_circuit)
		return FALSE;

	if (key1->is_circuit)
		return (key1->circ.circuit == key2->circ.circuit);
	else
		return (key1->circ.conv == key2->circ.conv);
}
/* init function, call from stream_init() */
static void init_stream_hash(void) {
	g_assert(stream_hash == NULL);
	stream_hash = g_hash_table_new(stream_hash_func,
		stream_compare_func);
}
typedef struct {
	fragment_head *fd_head;          /* the reassembled data, NULL
									 * until we add the last fragment */
	guint32 pdu_number;              /* Number of this PDU within the stream */

									 /* id of this pdu (globally unique) */
	guint32 id;
} stream_pdu_t;
struct stream {
	/* the key used to add this stream to stream_hash */
	struct stream_key *key;

	/* pdu to add the next fragment to, or NULL if we need to start
	* a new PDU.
	*/
	stream_pdu_t *current_pdu;

	/* number of PDUs added to this stream so far */
	guint32 pdu_counter;

	/* the framenumber and offset of the last fragment added;
	used for sanity-checking */
	guint32 lastfrag_framenum;
	guint32 lastfrag_offset;
};
typedef struct stream stream_t;
/* key */
typedef struct fragment_key {
	const stream_t *stream;
	guint32 framenum;
	guint32 offset;
} fragment_key_t;
/* hash func */
static guint fragment_hash_func(gconstpointer k)
{
	const fragment_key_t *key = (const fragment_key_t *)k;
	return (GPOINTER_TO_UINT(key->stream)) + ((guint)key->framenum) + ((guint)key->offset);
}

/* compare func */
static gboolean fragment_compare_func(gconstpointer a,
	gconstpointer b)
{
	const fragment_key_t *key1 = (const fragment_key_t *)a;
	const fragment_key_t *key2 = (const fragment_key_t *)b;
	return (key1->stream == key2->stream &&
		key1->framenum == key2->framenum &&
		key1->offset == key2->offset);
}
/* init function, call from stream_init() */
static void init_fragment_hash(void) {
	g_assert(fragment_hash == NULL);
	fragment_hash = g_hash_table_new(fragment_hash_func,
		fragment_compare_func);
}
/******************************************************************************
*
* PDU data
*/

/* pdu counter, for generating unique pdu ids */
static guint32 pdu_counter;

static void stream_init_pdu_data(void)
{
	pdu_counter = 0;
}
/*
* Table of functions for a reassembly table.
*/
typedef struct {
	/* Functions for fragment table */
	GHashFunc hash_func;				/* hash function */
	GEqualFunc equal_func;				/* comparison function */
	fragment_temporary_key temporary_key_func;	/* temporary key creation function */
	fragment_persistent_key persistent_key_func;	/* persistent key creation function */
	GDestroyNotify free_temporary_key_func;		/* temporary key destruction function */
	GDestroyNotify free_persistent_key_func;	/* persistent key destruction function */
} reassembly_table_functions;

typedef struct _reassembled_key {
	guint32 id;
	guint32 frame;
} reassembled_key;
static guint
reassembled_hash(gconstpointer k)
{
	const reassembled_key* key = (const reassembled_key*)k;

	return key->frame;
}

static gint
reassembled_equal(gconstpointer k1, gconstpointer k2)
{
	const reassembled_key* key1 = (const reassembled_key*)k1;
	const reassembled_key* key2 = (const reassembled_key*)k2;

	/*
	* We assume that the frame numbers are unlikely to be equal,
	* so we check them first.
	*/
	return key1->frame == key2->frame && key1->id == key2->id;
}

static void
reassembled_key_free(gpointer ptr)
{
	g_slice_free(reassembled_key, (reassembled_key *)ptr);
}
/*
* Initialize a reassembly table, with specified functions.
*/
void
reassembly_table_init(reassembly_table *table,
	const reassembly_table_functions *funcs)
{
	if (table->temporary_key_func == NULL)
		table->temporary_key_func = funcs->temporary_key_func;
	if (table->persistent_key_func == NULL)
		table->persistent_key_func = funcs->persistent_key_func;
	if (table->free_temporary_key_func == NULL)
		table->free_temporary_key_func = funcs->free_temporary_key_func;
	if (table->fragment_table != NULL) {
		/*
		* The fragment hash table exists.
		*
		* Remove all entries and free fragment data for each entry.
		*
		* The keys, and anything to which they point, are freed by
		* calling the table's key freeing function.  The values
		* are freed in free_all_fragments().
		*/
		g_hash_table_foreach_remove(table->fragment_table,
			free_all_fragments, NULL);
	}
	else {
		/* The fragment table does not exist. Create it */
		table->fragment_table = g_hash_table_new_full(funcs->hash_func,
			funcs->equal_func, funcs->free_persistent_key_func, NULL);
	}

	if (table->reassembled_table != NULL) {
		GPtrArray *allocated_fragments;

		/*
		* The reassembled-packet hash table exists.
		*
		* Remove all entries and free reassembled packet
		* data and key for each entry.
		*/

		allocated_fragments = g_ptr_array_new();
		g_hash_table_foreach_remove(table->reassembled_table,
			free_all_reassembled_fragments, allocated_fragments);

		g_ptr_array_foreach(allocated_fragments, free_fragments, NULL);
		g_ptr_array_free(allocated_fragments, TRUE);
	}
	else {
		/* The fragment table does not exist. Create it */
		table->reassembled_table = g_hash_table_new_full(reassembled_hash,
			reassembled_equal, reassembled_key_free, NULL);
	}
}
/*
* Tables of functions exported for the benefit of dissectors that
* don't need special items in their keys.
*/
WS_DLL_PUBLIC const reassembly_table_functions
addresses_reassembly_table_functions;
/* initialise the stream routines */
void stream_init(void)
{
	init_stream_hash();
	init_fragment_hash();
	stream_init_pdu_data();

	reassembly_table_init(&stream_reassembly_table,
		&addresses_reassembly_table_functions);
}
typedef enum _wmem_node_color_t {
	WMEM_NODE_COLOR_RED,
	WMEM_NODE_COLOR_BLACK
} wmem_node_color_t;
struct _wmem_tree_node_t {
	struct _wmem_tree_node_t *parent;
	struct _wmem_tree_node_t *left;
	struct _wmem_tree_node_t *right;

	const void *key;
	void *data;

	wmem_node_color_t color;
	gboolean          is_subtree;
	gboolean          is_removed;


};
typedef struct _wmem_tree_node_t wmem_tree_node_t;
struct _wmem_tree_t {
	wmem_allocator_t *master;
	wmem_allocator_t *allocator;
	wmem_tree_node_t *root;
	guint             master_cb_id;
	guint             slave_cb_id;

	void(*post_rotation_cb)(wmem_tree_node_t *);
};
typedef struct _wmem_tree_t wmem_tree_t;
static wmem_tree_t *registered_ct_tables = NULL;
struct pref_module {
	const char *name;           /**< name of module */
	const char *title;          /**< title of module (displayed in preferences list) */
	const char *description;    /**< Description of module (displayed in preferences notebook) */
	void(*apply_cb)(void);     /**< routine to call when preferences applied */
	GList *prefs;               /**< list of its preferences */
	struct pref_module *parent; /**< parent module */
	wmem_tree_t *submodules;    /**< list of its submodules */
	int numprefs;               /**< number of non-obsolete preferences */
	unsigned int prefs_changed_flags;    /**< Bitmask of the types of changes done by module preferences since we last checked */
	gboolean obsolete;          /**< if TRUE, this is a module that used to
								* exist but no longer does
								*/
	gboolean use_gui;           /**< Determines whether or not the module will use the generic
								* GUI interface/APIs with the preference value or if its own
								* independent GUI will be provided.  This allows all preferences
								* to have a common API for reading/writing, but not require them to
								* use simple GUI controls to change the options.  In general, the "general"
								* Wireshark preferences should have this set to FALSE, while the protocol
								* modules will have this set to TRUE */
	unsigned int effect_flags;  /**< Flags of types effected by preference (PREF_TYPE_DISSECTION, PREF_EFFECT_CAPTURE, etc).
								These flags will be set in all module's preferences on creation. Flags must be non-zero
								to ensure saving to disk */
};
typedef struct pref_module module_t;
/**
* Copy CB
* copy(dest, source, len)
*
* Used to duplicate the contents of one record to another.
* Optional, memcpy will be used if not given.
*/
typedef void* (*uat_copy_cb_t)(void *dest, const void *source, size_t len);
/**
* Update CB
* update(record,&error)
*
* Validates the contents of the record contents, to be called after any record
* fields had been updated (either from file or after modifications in the GUI).
*
* Optional, the record will be considered valid if the callback is omitted.
* It must return TRUE if the contents are considered valid and FALSE otherwise
* in which case the failure reason is set in 'error'. The error string will be
* freed by g_free.
*/
typedef gboolean(*uat_update_cb_t)(void *record, char **error);
/**
* Free CB
* free(record)
*
* Destroy the contents of a record, possibly freeing some fields.
* Do not free the container itself, this memory is owned by the UAT core.
* Optional if the record contains no pointers that need to be freed.
*/
typedef void(*uat_free_cb_t)(void *record);
/********
* Callbacks dealing with the entire table
********/

/*
* Post-Update CB
*
* To be called by the GUI code after to the table has being edited.
* Will be called once the user clicks the Apply or OK button
* optional
*/
typedef void(*uat_post_update_cb_t)(void);
/**
* Reset DB
*
* Used to free resources associated with a UAT loaded from file (e.g. post_update_cb)
* Optional.
*/

typedef void(*uat_reset_cb_t)(void);
typedef struct _uat_rep_t uat_rep_t;
typedef void(*uat_rep_free_cb_t)(uat_rep_t*);

/*******
* Callbacks for single fields (these deal with single values)
* the caller should provide one of these for every field!
********/

/*
* Check CB
* chk(record, ptr, len, chk_data, fld_data, &error)
*
* given an input string (ptr, len) checks if the value is OK for a field in the record.
* it will return TRUE if OK or else
* it will return FALSE and set *error to inform the user on what's
* wrong with the given input
* The error string must be allocated with g_malloc() or
* a routine that calls it.
* optional, if not given any input is considered OK and the set cb will be called
*/
typedef gboolean(*uat_fld_chk_cb_t)(void *record, const char *ptr, unsigned len, const void *chk_data, const void *fld_data, char **error);

/*
* Set Field CB
* set(record, ptr, len, set_data, fld_data)
*
* given an input string (ptr, len) sets the value of a field in the record,
* it is mandatory
*/
typedef void(*uat_fld_set_cb_t)(void *record, const char *ptr, unsigned len, const void *set_data, const void *fld_data);

/*
* Convert-to-string CB
* tostr(record, &out_ptr, &out_len, tostr_data, fld_data)
*
* given a record returns a string representation of the field
* mandatory
*/
typedef void(*uat_fld_tostr_cb_t)(void *record, char **out_ptr, unsigned *out_len, const void *tostr_data, const void *fld_data);
typedef void(*uat_rep_free_cb_t)(uat_rep_t*);
/***********
* Text Mode
*
* used for file and dialog representation of fields in columns,
* when the file is read it modifies the way the value is passed back to the fld_set_cb
* (see definition bellow for description)
***********/

typedef enum _uat_text_mode_t {
	PT_TXTMOD_NONE,
	/* not used */

	PT_TXTMOD_STRING,
	/*
	file:
	reads:
	,"\x20\x00\x30", as " \00",3 ("space nil zero" of length 3)
	,"", as "",0
	,, as NULL,0
	writes:
	,"\x20\x30\x00\x20", for " 0\0 ",4
	,"", for *, 0
	,, for NULL, *
	dialog:
	accepts \x?? and other escapes
	gets "",0 on empty string
	*/
	PT_TXTMOD_HEXBYTES,
	/*
	file:
	reads:
	,A1b2C3d4, as "\xa1\xb2\xc3\xd4",4
	,, as NULL,0
	writes:
	,, on NULL, *
	,a1b2c3d4, on "\xa1\xb2\xc3\xd4",4
	dialog:
	interprets the following input ... as ...:
	"a1b2c3d4" as "\xa1\xb2\xc3\xd4",4
	"a1 b2:c3d4" as "\xa1\xb2\xc3\xd4",4
	"" as NULL,0
	"invalid" as NULL,3
	"a1b" as NULL, 1
	*/
	PT_TXTMOD_ENUM,
	/* Read/Writes/displays the string value (not number!) */

	PT_TXTMOD_COLOR,
	/* Reads/Writes/display color in #RRGGBB format */

	PT_TXTMOD_FILENAME,
	/* processed like a PT_TXTMOD_STRING, but shows a filename dialog */
	PT_TXTMOD_DIRECTORYNAME,
	/* processed like a PT_TXTMOD_STRING, but shows a directory dialog */
	PT_TXTMOD_DISPLAY_FILTER,
	/* processed like a PT_TXTMOD_STRING, but verifies display filter */
	PT_TXTMOD_PROTO_FIELD,
	/* processed like a PT_TXTMOD_STRING, but verifies protocol field name (e.g tcp.flags.syn) */
	PT_TXTMOD_BOOL
	/* Displays a checkbox for value */
} uat_text_mode_t;
/*
* Fields
*
*
*/
typedef struct _uat_field_t {
	const char* name;
	const char* title;
	uat_text_mode_t mode;

	struct {
		uat_fld_chk_cb_t chk;
		uat_fld_set_cb_t set;
		uat_fld_tostr_cb_t tostr;
	} cb;

	struct {
		const void* chk;
		const void* set;
		const void* tostr;
	} cbdata;

	const void* fld_data;

	const char* desc;
	struct _fld_data_t* priv;
} uat_field_t;
struct epan_uat {
	char* name;
	size_t record_size;
	char* filename;
	gboolean from_profile;
	char* help;
	guint flags;
	void** user_ptr;    /**< Pointer to a dissector variable where an array of valid records are stored. */
	guint* nrows_p;     /**< Pointer to a dissector variable where the number of valid records in user_ptr are written. */
	uat_copy_cb_t copy_cb;
	uat_update_cb_t update_cb;
	uat_free_cb_t free_cb;
	uat_post_update_cb_t post_update_cb;
	uat_reset_cb_t reset_cb;

	uat_field_t* fields;
	guint ncols;
	GArray* user_data;  /**< An array of valid records that will be exposed to the dissector. */
	GArray* raw_data;   /**< An array of records containing possibly invalid data. For internal use only. */
	GArray* valid_data; /**< An array of booleans describing whether the records in 'raw_data' are valid or not. */
	gboolean changed;
	uat_rep_t* rep;
	uat_rep_free_cb_t free_rep;
	gboolean loaded;
	gboolean from_global;
};
typedef struct epan_uat uat_t;
typedef struct hf_register_info {
	int				*p_id;	/**< written to by register() function */
	header_field_info		hfinfo;	/**< the field info to be registered */
} hf_register_info;
static int hf_expert_msg = -1;
/*
* Note that this enum values are parsed in make-init-lua.pl so make sure
* any changes here still makes valid entries in init.lua.
*/
typedef enum {
	/* Integral types */
	BASE_NONE = 0,   /**< none */
	BASE_DEC = 1,   /**< decimal */
	BASE_HEX = 2,   /**< hexadecimal */
	BASE_OCT = 3,   /**< octal */
	BASE_DEC_HEX = 4,   /**< decimal (hexadecimal) */
	BASE_HEX_DEC = 5,   /**< hexadecimal (decimal) */
	BASE_CUSTOM = 6,   /**< call custom routine (in ->strings) to format */

	/* Float types */
	BASE_FLOAT = BASE_NONE, /**< decimal-format float */

	/* String types */
	STR_ASCII = 0,   /**< shows non-printable ASCII characters as C-style escapes */
																/* XXX, support for format_text_wsp() ? */
	STR_UNICODE = 7,   /**< shows non-printable UNICODE characters as \\uXXXX (XXX for now non-printable characters display depends on UI) */

	/* Byte separators */
	SEP_DOT = 8,   /**< hexadecimal bytes with a period (.) between each byte */
																				   SEP_DASH = 9,   /**< hexadecimal bytes with a dash (-) between each byte */
	SEP_COLON = 10,  /**< hexadecimal bytes with a colon (:) between each byte */
	SEP_SPACE = 11,  /**< hexadecimal bytes with a space between each byte */

	/* Address types */
	BASE_NETMASK = 12,  /**< Used for IPv4 address that shouldn't be resolved (like for netmasks) */

	/* Port types */
	BASE_PT_UDP = 13,  /**< UDP port */
	BASE_PT_TCP = 14,  /**< TCP port */
	BASE_PT_DCCP = 15,  /**< DCCP port */
	BASE_PT_SCTP = 16,  /**< SCTP port */

	/* OUI types */
	BASE_OUI = 17   /**< OUI resolution */

} field_display_e;
static int hf_expert_group = -1;
static int hf_expert_severity = -1;
struct _value_string;
#define VALS(x)	(const struct _value_string*)(x)
/* VALUE TO STRING MATCHING */

typedef struct _value_string {
	guint32      value;
	const gchar *strptr;
} value_string;
/* do not modify the PI_GROUP_MASK name - it's used by make-init-lua.pl */
/* expert "event groups" */
#define PI_GROUP_MASK           0xFF000000	/**< mask usually for internal use only! */
/** The protocol field has a bad checksum, usually uses PI_WARN severity */
#define PI_CHECKSUM             0x01000000
/** The protocol field indicates a sequence problem (e.g. TCP window is zero) */
#define PI_SEQUENCE             0x02000000
/** The protocol field indicates a bad application response code (e.g. HTTP 404), usually PI_NOTE severity */
#define PI_RESPONSE_CODE        0x03000000
/** The protocol field indicates an application request (e.g. File Handle == xxxx), usually PI_CHAT severity */
#define PI_REQUEST_CODE         0x04000000
/** The data is undecoded, the protocol dissection is incomplete here, usually PI_WARN severity */
#define PI_UNDECODED            0x05000000
/** The protocol field indicates a reassemble (e.g. DCE/RPC defragmentation), usually PI_CHAT severity (or PI_ERROR) */
#define PI_REASSEMBLE           0x06000000
/** The packet data is malformed, the dissector has "given up", usually PI_ERROR severity */
#define PI_MALFORMED            0x07000000
/** A generic debugging message (shouldn't remain in production code!), usually PI_ERROR severity */
#define PI_DEBUG                0x08000000
/** The protocol field violates a protocol specification, usually PI_WARN severity */
#define PI_PROTOCOL             0x09000000
/** The protocol field indicates a security problem (e.g. insecure implementation) */
#define PI_SECURITY             0x0a000000
/** The protocol field indicates a packet comment */
#define PI_COMMENTS_GROUP       0x0b000000
/** The protocol field indicates a decryption problem */
#define PI_DECRYPTION           0x0c000000
/** The protocol field has incomplete data, decode based on assumed value */
#define PI_ASSUMPTION           0x0d000000
/** The protocol field has been deprecated, usually PI_NOTE severity */
#define PI_DEPRECATED           0x0e000000

const value_string expert_group_vals[] = {
	{ PI_CHECKSUM,          "Checksum" },
	{ PI_SEQUENCE,          "Sequence" },
	{ PI_RESPONSE_CODE,     "Response" },
	{ PI_REQUEST_CODE,      "Request" },
	{ PI_UNDECODED,         "Undecoded" },
	{ PI_REASSEMBLE,        "Reassemble" },
	{ PI_MALFORMED,         "Malformed" },
	{ PI_DEBUG,             "Debug" },
	{ PI_PROTOCOL,          "Protocol" },
	{ PI_SECURITY,          "Security" },
	{ PI_COMMENTS_GROUP,    "Comment" },
	{ PI_DECRYPTION,        "Decryption" },
	{ PI_ASSUMPTION,        "Assumption" },
	{ PI_DEPRECATED,        "Deprecated" },
	{ 0, NULL }
};
/** Packet comment */
#define PI_COMMENT              0x00100000
/** Usual workflow, e.g. TCP connection establishing */
#define PI_CHAT                 0x00200000
/** Notable messages, e.g. an application returned an "unusual" error code like HTTP 404 */
#define PI_NOTE                 0x00400000
/** Warning, e.g. application returned an "unusual" error code */
#define PI_WARN                 0x00600000
/** Serious problems, e.g. a malformed packet */
#define PI_ERROR                0x00800000

const value_string expert_severity_vals[] = {
	{ PI_ERROR,             "Error" },
	{ PI_WARN,              "Warning" },
	{ PI_NOTE,              "Note" },
	{ PI_CHAT,              "Chat" },
	{ PI_COMMENT,           "Comment" },
	{ 1,                    "Ok" },
	{ 0, NULL }
};
gboolean uat_fld_chk_str(void* u1 _U_, const char* strptr, guint len _U_, const void* u2 _U_, const void* u3 _U_, char** err)
{
	if (strptr == NULL) {
		*err = g_strdup("NULL pointer");
		return FALSE;
	}

	*err = NULL;
	return TRUE;
}
gboolean uat_fld_chk_enum(void* u1 _U_, const char* strptr, guint len, const void* v, const void* u3 _U_, char** err) {
	char* str = g_strndup(strptr, len);
	guint i;
	const value_string* vs = (const value_string *)v;

	for (i = 0; vs[i].strptr; i++) {
		if (g_strcmp0(vs[i].strptr, str) == 0) {
			*err = NULL;
			g_free(str);
			return TRUE;
		}
	}

	*err = g_strdup_printf("invalid value: %s", str);
	g_free(str);
	return FALSE;
}
/*
* CSTRING macros,
*    a simple c-string contained in (((rec_t*)rec)->(field_name))
*/
#define UAT_CSTRING_CB_DEF(basename,field_name,rec_t) \
static void basename ## _ ## field_name ## _set_cb(void* rec, const char* buf, guint len, const void* UNUSED_PARAMETER(u1), const void* UNUSED_PARAMETER(u2)) {\
    char* new_buf = g_strndup(buf,len); \
	g_free((((rec_t*)rec)->field_name)); \
	(((rec_t*)rec)->field_name) = new_buf; } \
static void basename ## _ ## field_name ## _tostr_cb(void* rec, char** out_ptr, unsigned* out_len, const void* UNUSED_PARAMETER(u1), const void* UNUSED_PARAMETER(u2)) {\
		if (((rec_t*)rec)->field_name ) { \
			*out_ptr = g_strdup((((rec_t*)rec)->field_name)); \
			*out_len = (unsigned)strlen((((rec_t*)rec)->field_name)); \
		} else { \
			*out_ptr = g_strdup(""); *out_len = 0; } }

struct _wmem_array_t {
	wmem_allocator_t *allocator;

	guint8 *buf;

	gsize elem_size;

	guint elem_count;
	guint alloc_count;

	gboolean null_terminated;
};
typedef struct _wmem_array_t wmem_array_t;
struct _attr_reg_data {
	wmem_array_t *hf;
	const gchar *basename;
};
/* Stuff for generation/handling of fields */
typedef struct {
	gchar* call_path; /* equals to grpc :path, for example: "/helloworld.Greeter/SayHello" */
	guint direction_type; /* 0: request, 1: response */
	guint field_type; /* type of field, refer to protobuf_field_type vals. */
	gchar* field_name; /* field name, will display in tree */
	guint field_number; /* field number in .proto file*/
} protobuf_field_t;
/*
* Fields
*
*
*/

#define FLDFILL NULL
#define UAT_FLD_CSTRING(basename,field_name,title,desc) \
	{#field_name, title, PT_TXTMOD_STRING,{uat_fld_chk_str,basename ## _ ## field_name ## _set_cb,basename ## _ ## field_name ## _tostr_cb},{0,0,0},0,desc,FLDFILL}
#define UAT_FLD_VS(basename,field_name,title,enum,desc) \
	{#field_name, title, PT_TXTMOD_ENUM,{uat_fld_chk_enum,basename ## _ ## field_name ## _set_cb,basename ## _ ## field_name ## _tostr_cb},{&(enum),&(enum),&(enum)},&(enum),desc,FLDFILL}
#define UAT_END_FIELDS {NULL,NULL,PT_TXTMOD_NONE,{0,0,0},{0,0,0},0,0,FLDFILL}

/*----------------------------------------------------------------------------*/
/* UAT for customizing severity levels.                                       */
/*----------------------------------------------------------------------------*/
typedef struct
{
	char    *field;
	guint32  severity;
} expert_level_entry_t;

static expert_level_entry_t *uat_expert_entries = NULL;

static int ett_expert = -1;
static int ett_subexpert = -1;
static int expert_tap = -1;

typedef struct _tap_dissector_t {
	struct _tap_dissector_t *next;
	char *name;
} tap_dissector_t;
static tap_dissector_t *tap_dissector_list = NULL;
/* This function registers that a dissector has the packet tap ability
available.  The name parameter is the name of this tap and extensions can
use open_tap(char *name,... to specify that it wants to receive packets/
events from this tap.

This function is only to be called once, when the dissector initializes.

The return value from this call is later used as a parameter to the
tap_packet(unsigned int *tap_id,...
call so that the tap subsystem knows to which tap point this tapped
packet is associated.
*/
int
register_tap(const char *name)
{
	tap_dissector_t *td, *tdl = NULL, *tdl_prev = NULL;
	int i = 0;

	if (tap_dissector_list) {
		/* Check if we allready have the name registered, if it is return the tap_id of that tap.
		* After the for loop tdl_prev will point to the last element of the list, add the new one there.
		*/
		for (i = 1, tdl = tap_dissector_list; tdl; i++, tdl_prev = tdl, tdl = tdl->next) {
			if (!strcmp(tdl->name, name)) {
				return i;
			}
		}
		tdl = tdl_prev;
	}

	td = (tap_dissector_t *)g_malloc(sizeof(tap_dissector_t));
	td->next = NULL;
	td->name = g_strdup(name);

	if (!tap_dissector_list) {
		tap_dissector_list = td;
		i = 1;
	}
	else {
		tdl->next = td;
	}
	return i;
}
/* proto_expert cannot be static because it's referenced in the
* print routines
*/
int proto_expert = -1;
/* Name hashtables for fast detection of duplicate names */
static GHashTable* proto_names = NULL;
static GHashTable* proto_short_names = NULL;
static GHashTable* proto_filter_names = NULL;
/* Structure for information about a protocol */
struct _protocol {
	const char *name;               /* long description */
	const char *short_name;         /* short description */
	const char *filter_name;        /* name of this protocol in filters */
	GPtrArray  *fields;             /* fields for this protocol */
	int         proto_id;           /* field ID for this protocol */
	gboolean    is_enabled;         /* TRUE if protocol is enabled */
	gboolean    enabled_by_default; /* TRUE if protocol is enabled by default */
	gboolean    can_toggle;         /* TRUE if is_enabled can be changed */
	int         parent_proto_id;    /* Used to identify "pino"s (Protocol In Name Only).
									For dissectors that need a protocol name so they
									can be added to a dissector table, but use the
									parent_proto_id for things like enable/disable */
	GList      *heur_list;          /* Heuristic dissectors associated with this protocol */
};
typedef struct _protocol protocol_t;
/* chars allowed in field abbrev: alphanumerics, '-', "_", and ".". */
static
const guint8 fld_abbrev_chars[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00-0x0F */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10-0x1F */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, /* 0x20-0x2F '-', '.'	   */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, /* 0x30-0x3F '0'-'9'	   */
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x40-0x4F 'A'-'O'	   */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /* 0x50-0x5F 'P'-'Z', '_' */
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x60-0x6F 'a'-'o'	   */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, /* 0x70-0x7F 'p'-'z'	   */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x80-0x8F */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x90-0x9F */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xA0-0xAF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xB0-0xBF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xC0-0xCF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xD0-0xDF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xE0-0xEF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xF0-0xFF */
};
guchar
proto_check_field_name(const gchar *field_name)
{
	const char *p = field_name;
	guchar c = '.', lastc;

	do {
		lastc = c;
		c = *(p++);
		/* Leading '.' or substring ".." are disallowed. */
		if (c == '.' && lastc == '.') {
			break;
		}
	} while (fld_abbrev_chars[c]);

	/* Trailing '.' is disallowed. */
	if (lastc == '.') {
		return '.';
	}
	return c;
}
static void
check_valid_filter_name_or_fail(const char *filter_name)
{
	gboolean found_invalid = proto_check_field_name(filter_name);

	/* Additionally forbid upper case characters. */
	if (!found_invalid) {
		for (guint i = 0; filter_name[i]; i++) {
			if (g_ascii_isupper(filter_name[i])) {
				found_invalid = TRUE;
				break;
			}
		}
	}

	if (found_invalid) {
		g_error("Protocol filter name \"%s\" has one or more invalid characters."
			" Allowed are lower characters, digits, '-', '_' and non-repeating '.'."
			" This might be caused by an inappropriate plugin or a development error.", filter_name);
	}
}
static GList *protocols = NULL;
/*Following constants have to be ORed with a field_display_e when dissector
* want to use specials value - string MACROs for a header_field_info */
#define BASE_RANGE_STRING       0x0100
#define BASE_EXT_STRING         0x0200
#define BASE_VAL64_STRING       0x0400
#define BASE_ALLOW_ZERO         0x0800  /**< Display <none> instead of <MISSING> for zero sized byte array */
#define BASE_UNIT_STRING        0x1000  /**< Add unit text to the field value */
#define BASE_NO_DISPLAY_VALUE   0x2000  /**< Just display the field name with no value.  Intended for
byte arrays or header fields above a subtree */
#define BASE_PROTOCOL_INFO      0x4000  /**< protocol_t in [FIELDCONVERT].  Internal use only. */
#define BASE_SPECIAL_VALS    0x8000  /**< field will not display "Unknown" if value_string match is not found */
/* Returns a string representing the name of the type. Useful
* for glossary production. */
/* Keep track of ftype_t's via their ftenum number */
static ftype_t* type_list[FT_NUM_TYPES];
/* Given an ftenum number, return an ftype_t* */
#define FTYPE_LOOKUP(ftype, result)	\
	/* Check input */		\
	g_assert(ftype < FT_NUM_TYPES);	\
	result = type_list[ftype];

const char*
ftype_name(enum ftenum ftype)
{
	ftype_t	*ft;

	FTYPE_LOOKUP(ftype, ft);
	return ft->name;
}
/** FIELD_DISPLAY_E_MASK selects the field_display_e value. */
#define FIELD_DISPLAY_E_MASK 0xFF

#define FIELD_DISPLAY(d) ((d) & FIELD_DISPLAY_E_MASK)
typedef enum {
	/* Start at 1000 to avoid duplicating the values used in field_display_e */
	ABSOLUTE_TIME_LOCAL = 1000,	/* local time in our time zone, with month and day */
	ABSOLUTE_TIME_UTC,	/* UTC, with month and day */
	ABSOLUTE_TIME_DOY_UTC	/* UTC, with 1-origin day-of-year */
} absolute_time_display_e;
static const value_string hf_display[] = {
	{ BASE_NONE,			  "BASE_NONE" },
	{ BASE_DEC,			  "BASE_DEC" },
	{ BASE_HEX,			  "BASE_HEX" },
	{ BASE_OCT,			  "BASE_OCT" },
	{ BASE_DEC_HEX,			  "BASE_DEC_HEX" },
	{ BASE_HEX_DEC,			  "BASE_HEX_DEC" },
	{ BASE_CUSTOM,			  "BASE_CUSTOM" },
	{ BASE_NONE | BASE_RANGE_STRING,    "BASE_NONE|BASE_RANGE_STRING" },
	{ BASE_DEC | BASE_RANGE_STRING,     "BASE_DEC|BASE_RANGE_STRING" },
	{ BASE_HEX | BASE_RANGE_STRING,     "BASE_HEX|BASE_RANGE_STRING" },
	{ BASE_OCT | BASE_RANGE_STRING,     "BASE_OCT|BASE_RANGE_STRING" },
	{ BASE_DEC_HEX | BASE_RANGE_STRING, "BASE_DEC_HEX|BASE_RANGE_STRING" },
	{ BASE_HEX_DEC | BASE_RANGE_STRING, "BASE_HEX_DEC|BASE_RANGE_STRING" },
	{ BASE_CUSTOM | BASE_RANGE_STRING,  "BASE_CUSTOM|BASE_RANGE_STRING" },
	{ BASE_NONE | BASE_VAL64_STRING,    "BASE_NONE|BASE_VAL64_STRING" },
	{ BASE_DEC | BASE_VAL64_STRING,     "BASE_DEC|BASE_VAL64_STRING" },
	{ BASE_HEX | BASE_VAL64_STRING,     "BASE_HEX|BASE_VAL64_STRING" },
	{ BASE_OCT | BASE_VAL64_STRING,     "BASE_OCT|BASE_VAL64_STRING" },
	{ BASE_DEC_HEX | BASE_VAL64_STRING, "BASE_DEC_HEX|BASE_VAL64_STRING" },
	{ BASE_HEX_DEC | BASE_VAL64_STRING, "BASE_HEX_DEC|BASE_VAL64_STRING" },
	{ BASE_CUSTOM | BASE_VAL64_STRING,  "BASE_CUSTOM|BASE_VAL64_STRING" },
	/* Alias: BASE_NONE { BASE_FLOAT,			"BASE_FLOAT" }, */
	/* Alias: BASE_NONE { STR_ASCII,			  "STR_ASCII" }, */
	{ STR_UNICODE,			  "STR_UNICODE" },
	{ ABSOLUTE_TIME_LOCAL,		  "ABSOLUTE_TIME_LOCAL" },
	{ ABSOLUTE_TIME_UTC,		  "ABSOLUTE_TIME_UTC" },
	{ ABSOLUTE_TIME_DOY_UTC,	  "ABSOLUTE_TIME_DOY_UTC" },
	{ BASE_PT_UDP,			  "BASE_PT_UDP" },
	{ BASE_PT_TCP,			  "BASE_PT_TCP" },
	{ BASE_PT_DCCP,			  "BASE_PT_DCCP" },
	{ BASE_PT_SCTP,			  "BASE_PT_SCTP" },
	{ BASE_OUI,			  "BASE_OUI" },
	{ 0,				  NULL } 
};
/* Tries to match val against each element in the value_string array vs.
Returns the associated string ptr, and sets "*idx" to the index in
that table, on a match, and returns NULL, and sets "*idx" to -1,
on failure. */
const gchar *
try_val_to_str_idx(const guint32 val, const value_string *vs, gint *idx)
{
	gint i = 0;

	DISSECTOR_ASSERT(idx != NULL);

	if (vs) {
		while (vs[i].strptr) {
			if (vs[i].value == val) {
				*idx = i;
				return(vs[i].strptr);
			}
			i++;
		}
	}

	*idx = -1;
	return NULL;
}
/* Like try_val_to_str_idx(), but doesn't return the index. */
const gchar *
try_val_to_str(const guint32 val, const value_string *vs)
{
	gint ignore_me;
	return try_val_to_str_idx(val, vs, &ignore_me);
}
gchar *
wmem_strdup(wmem_allocator_t *allocator, const gchar *src)
{
	size_t len;

	/* If the string is NULL, just return the string "<NULL>" so that the
	* callers don't have to bother checking it. */
	if (!src) {
		src = "<NULL>";
	}

	len = strlen(src) + 1; /* +1 for the null-terminator */

	return (gchar *)memcpy(wmem_alloc(allocator, len), src, len);
}
#ifndef _WIN32
#define WMEM_STRDUP_VPRINTF_DEFAULT_BUFFER 80
gchar *
wmem_strdup_vprintf(wmem_allocator_t *allocator, const gchar *fmt, va_list ap)
{
	va_list ap2;
	gchar *dst;
	int needed_len;

	G_VA_COPY(ap2, ap);

	/* needed_len = g_printf_string_upper_bound(fmt, ap2); */

	dst = (gchar *)wmem_alloc(allocator, WMEM_STRDUP_VPRINTF_DEFAULT_BUFFER);

	/* Returns: the number of characters which would be produced if the buffer was large enough
	* (not including the null, for which we add +1 ourselves). */
	needed_len = g_vsnprintf(dst, (gulong)WMEM_STRDUP_VPRINTF_DEFAULT_BUFFER, fmt, ap2) + 1;
	va_end(ap2);

	if (needed_len > WMEM_STRDUP_VPRINTF_DEFAULT_BUFFER) {
		wmem_free(allocator, dst);
		dst = (gchar *)wmem_alloc(allocator, needed_len);
		G_VA_COPY(ap2, ap);
		g_vsnprintf(dst, (gulong)needed_len, fmt, ap2);
		va_end(ap2);
	}

	return dst;
}
#else /* _WIN32 */
/*
* GLib's v*printf routines are surprisingly slow on Windows, at least with
* GLib 2.40.0. This appears to be due to GLib using the gnulib version of
* vasnprintf when compiled under MinGW. If GLib ever ends up using the
* native Windows v*printf routines this can be removed.
*/
gchar *
wmem_strdup_vprintf(wmem_allocator_t *allocator, const gchar *fmt, va_list ap)
{
	va_list ap2;
	gchar *dst;
	int needed_len;

	G_VA_COPY(ap2, ap);

	needed_len = _vscprintf(fmt, ap2) + 1;

	dst = (gchar *)wmem_alloc(allocator, needed_len);

	vsprintf_s(dst, needed_len, fmt, ap2);

	va_end(ap2);

	return dst;
}
#endif /* _WIN32 */
gchar *
wmem_strdup_printf(wmem_allocator_t *allocator, const gchar *fmt, ...)
{
	va_list ap;
	gchar *dst;

	va_start(ap, fmt);
	dst = wmem_strdup_vprintf(allocator, fmt, ap);
	va_end(ap);

	return dst;
}
gchar *
val_to_str_wmem(wmem_allocator_t *scope, const guint32 val, const value_string *vs, const char *fmt)
{
	const gchar *ret;

	DISSECTOR_ASSERT(fmt != NULL);

	ret = try_val_to_str(val, vs);
	if (ret != NULL)
		return wmem_strdup(scope, ret);

	return wmem_strdup_printf(scope, fmt, val);
}
/** BASE_PT_ values display decimal and transport port service name */
#define IS_BASE_PORT(b) (((b)==BASE_PT_UDP||(b)==BASE_PT_TCP||(b)==BASE_PT_DCCP||(b)==BASE_PT_SCTP))

/* temporary function containing assert part for easier profiling */
static void
tmp_fld_check_assert(header_field_info *hfinfo)
{
	gchar* tmp_str;

	/* The field must have a name (with length > 0) */
	if (!hfinfo->name || !hfinfo->name[0]) {
		if (hfinfo->abbrev)
			/* Try to identify the field */
			g_error("Field (abbrev='%s') does not have a name\n",
				hfinfo->abbrev);
		else
			/* Hum, no luck */
			g_error("Field does not have a name (nor an abbreviation)\n");
	}

	/* fields with an empty string for an abbreviation aren't filterable */
	if (!hfinfo->abbrev || !hfinfo->abbrev[0])
		g_error("Field '%s' does not have an abbreviation\n", hfinfo->name);

	/*  These types of fields are allowed to have value_strings,
	*  true_false_strings or a protocol_t struct
	*/
	if (hfinfo->strings != NULL) {
		switch (hfinfo->type) {
		case FT_CHAR:
		case FT_UINT8:
		case FT_UINT16:
		case FT_UINT24:
		case FT_UINT32:
		case FT_UINT40:
		case FT_UINT48:
		case FT_UINT56:
		case FT_UINT64:
		case FT_INT8:
		case FT_INT16:
		case FT_INT24:
		case FT_INT32:
		case FT_INT40:
		case FT_INT48:
		case FT_INT56:
		case FT_INT64:
		case FT_BOOLEAN:
		case FT_PROTOCOL:
		case FT_FRAMENUM:
			break;
		case FT_FLOAT:
		case FT_DOUBLE:
			//allowed to support string if its a unit decsription
			if (hfinfo->display & BASE_UNIT_STRING)
				break;

			//fallthrough
		case FT_BYTES:
			//allowed to support string if its a protocol (for pinos)
			if (hfinfo->display & BASE_PROTOCOL_INFO)
				break;

			//fallthrough
		default:
			g_error("Field '%s' (%s) has a 'strings' value but is of type %s"
				" (which is not allowed to have strings)\n",
				hfinfo->name, hfinfo->abbrev, ftype_name(hfinfo->type));
		}
	}

	/* TODO: This check may slow down startup, and output quite a few warnings.
	It would be good to be able to enable this (and possibly other checks?)
	in non-release builds.   */
#if ENABLE_CHECK_FILTER
	/* Check for duplicate value_string values.
	There are lots that have the same value *and* string, so for now only
	report those that have same value but different string. */
	if ((hfinfo->strings != NULL) &&
		!(hfinfo->display & BASE_RANGE_STRING) &&
		!(hfinfo->display & BASE_UNIT_STRING) &&
		!((hfinfo->display & FIELD_DISPLAY_E_MASK) == BASE_CUSTOM) &&
		(
		(hfinfo->type == FT_CHAR) ||
			(hfinfo->type == FT_UINT8) ||
			(hfinfo->type == FT_UINT16) ||
			(hfinfo->type == FT_UINT24) ||
			(hfinfo->type == FT_UINT32) ||
			(hfinfo->type == FT_INT8) ||
			(hfinfo->type == FT_INT16) ||
			(hfinfo->type == FT_INT24) ||
			(hfinfo->type == FT_INT32))) {

		int n, m;
		const value_string *start_values;
		const value_string *current;

		if (hfinfo->display & BASE_EXT_STRING)
			start_values = VALUE_STRING_EXT_VS_P(((const value_string_ext*)hfinfo->strings));
		else
			start_values = (const value_string*)hfinfo->strings;
		current = start_values;

		for (n = 0; current; n++, current++) {
			/* Drop out if we reached the end. */
			if ((current->value == 0) && (current->strptr == NULL)) {
				break;
			}

			/* Check value against all previous */
			for (m = 0; m < n; m++) {
				/* There are lots of duplicates with the same string,
				so only report if different... */
				if ((start_values[m].value == current->value) &&
					(strcmp(start_values[m].strptr, current->strptr) != 0)) {
					ws_g_warning("Field '%s' (%s) has a conflicting entry in its"
						" value_string: %u is at indices %u (%s) and %u (%s)\n",
						hfinfo->name, hfinfo->abbrev,
						current->value, m, start_values[m].strptr, n, current->strptr);
				}
			}
		}
	}
#endif


	switch (hfinfo->type) {

	case FT_CHAR:
		/*  Require the char type to have BASE_HEX, BASE_OCT,
		*  BASE_CUSTOM, or BASE_NONE as its base.
		*
		*  If the display value is BASE_NONE and there is a
		*  strings conversion then the dissector writer is
		*  telling us that the field's numerical value is
		*  meaningless; we'll avoid showing the value to the
		*  user.
		*/
		switch (FIELD_DISPLAY(hfinfo->display)) {
		case BASE_HEX:
		case BASE_OCT:
		case BASE_CUSTOM: /* hfinfo_numeric_value_format() treats this as decimal */
			break;
		case BASE_NONE:
			if (hfinfo->strings == NULL)
				g_error("Field '%s' (%s) is an integral value (%s)"
					" but is being displayed as BASE_NONE but"
					" without a strings conversion",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type));
			break;
		default:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			g_error("Field '%s' (%s) is a character value (%s)"
				" but is being displayed as %s\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->display & BASE_UNIT_STRING) {
			g_error("Field '%s' (%s) is a character value (%s) but has a unit string\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		}
		break;
	case FT_INT8:
	case FT_INT16:
	case FT_INT24:
	case FT_INT32:
	case FT_INT40:
	case FT_INT48:
	case FT_INT56:
	case FT_INT64:
		/*	Hexadecimal and octal are, in printf() and everywhere
		*	else, unsigned so don't allow dissectors to register a
		*	signed field to be displayed unsigned.  (Else how would
		*	we display negative values?)
		*/
		switch (FIELD_DISPLAY(hfinfo->display)) {
		case BASE_HEX:
		case BASE_OCT:
		case BASE_DEC_HEX:
		case BASE_HEX_DEC:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is signed (%s) but is being displayed unsigned (%s)\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}
		/* FALL THROUGH */
	case FT_UINT8:
	case FT_UINT16:
	case FT_UINT24:
	case FT_UINT32:
	case FT_UINT40:
	case FT_UINT48:
	case FT_UINT56:
	case FT_UINT64:
		if (IS_BASE_PORT(hfinfo->display)) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			if (hfinfo->type != FT_UINT16) {
				g_error("Field '%s' (%s) has 'display' value %s but it can only be used with FT_UINT16, not %s\n",
					hfinfo->name, hfinfo->abbrev,
					tmp_str, ftype_name(hfinfo->type));
			}
			if (hfinfo->strings != NULL) {
				g_error("Field '%s' (%s) is an %s (%s) but has a strings value\n",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type), tmp_str);
			}
			if (hfinfo->bitmask != 0) {
				g_error("Field '%s' (%s) is an %s (%s) but has a bitmask\n",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type), tmp_str);
			}
			wmem_free(NULL, tmp_str);
			break;
		}

		if (hfinfo->display == BASE_OUI) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			if (hfinfo->type != FT_UINT24) {
				g_error("Field '%s' (%s) has 'display' value %s but it can only be used with FT_UINT24, not %s\n",
					hfinfo->name, hfinfo->abbrev,
					tmp_str, ftype_name(hfinfo->type));
			}
			if (hfinfo->strings != NULL) {
				g_error("Field '%s' (%s) is an %s (%s) but has a strings value\n",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type), tmp_str);
			}
			if (hfinfo->bitmask != 0) {
				g_error("Field '%s' (%s) is an %s (%s) but has a bitmask\n",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type), tmp_str);
			}
			wmem_free(NULL, tmp_str);
			break;
		}

		/*  Require integral types (other than frame number,
		*  which is always displayed in decimal) to have a
		*  number base.
		*
		*  If the display value is BASE_NONE and there is a
		*  strings conversion then the dissector writer is
		*  telling us that the field's numerical value is
		*  meaningless; we'll avoid showing the value to the
		*  user.
		*/
		switch (FIELD_DISPLAY(hfinfo->display)) {
		case BASE_DEC:
		case BASE_HEX:
		case BASE_OCT:
		case BASE_DEC_HEX:
		case BASE_HEX_DEC:
		case BASE_CUSTOM: /* hfinfo_numeric_value_format() treats this as decimal */
			break;
		case BASE_NONE:
			if (hfinfo->strings == NULL) {
				g_error("Field '%s' (%s) is an integral value (%s)"
					" but is being displayed as BASE_NONE but"
					" without a strings conversion",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type));
			}
			if (hfinfo->display & BASE_SPECIAL_VALS) {
				g_error("Field '%s' (%s) is an integral value (%s)"
					" that is being displayed as BASE_NONE but"
					" with BASE_SPECIAL_VALS",
					hfinfo->name, hfinfo->abbrev,
					ftype_name(hfinfo->type));
			}
			break;

		default:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			g_error("Field '%s' (%s) is an integral value (%s)"
				" but is being displayed as %s\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}
		break;
	case FT_BYTES:
	case FT_UINT_BYTES:
		/*  Require bytes to have a "display type" that could
		*  add a character between displayed bytes.
		*/
		switch (FIELD_DISPLAY(hfinfo->display)) {
		case BASE_NONE:
		case SEP_DOT:
		case SEP_DASH:
		case SEP_COLON:
		case SEP_SPACE:
			break;
		default:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is an byte array but is being displayed as %s instead of BASE_NONE, SEP_DOT, SEP_DASH, SEP_COLON, or SEP_SPACE\n",
				hfinfo->name, hfinfo->abbrev, tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		//allowed to support string if its a protocol (for pinos)
		if ((hfinfo->strings != NULL) && (!(hfinfo->display & BASE_PROTOCOL_INFO)))
			g_error("Field '%s' (%s) is an %s but has a strings value\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;

	case FT_PROTOCOL:
	case FT_FRAMENUM:
		if (hfinfo->display != BASE_NONE) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is an %s but is being displayed as %s instead of BASE_NONE\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;

	case FT_BOOLEAN:
		break;

	case FT_ABSOLUTE_TIME:
		if (!(hfinfo->display == ABSOLUTE_TIME_LOCAL ||
			hfinfo->display == ABSOLUTE_TIME_UTC ||
			hfinfo->display == ABSOLUTE_TIME_DOY_UTC)) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is a %s but is being displayed as %s instead of as a time\n",
				hfinfo->name, hfinfo->abbrev, ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;

	case FT_STRING:
	case FT_STRINGZ:
	case FT_UINT_STRING:
	case FT_STRINGZPAD:
		switch (hfinfo->display) {
		case STR_ASCII:
		case STR_UNICODE:
			break;

		default:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			g_error("Field '%s' (%s) is an string value (%s)"
				" but is being displayed as %s\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
		}

		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		if (hfinfo->strings != NULL)
			g_error("Field '%s' (%s) is an %s but has a strings value\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;

	case FT_IPv4:
		switch (hfinfo->display) {
		case BASE_NONE:
		case BASE_NETMASK:
			break;

		default:
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Unknown: 0x%x)");
			g_error("Field '%s' (%s) is an IPv4 value (%s)"
				" but is being displayed as %s\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type), tmp_str);
			wmem_free(NULL, tmp_str);
			break;
		}
		break;
	case FT_FLOAT:
	case FT_DOUBLE:
		if (FIELD_DISPLAY(hfinfo->display) != BASE_NONE) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is an %s but is being displayed as %s instead of BASE_NONE\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type),
				tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		if ((hfinfo->strings != NULL) && (!(hfinfo->display & BASE_UNIT_STRING)))
			g_error("Field '%s' (%s) is an %s but has a strings value\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;
	default:
		if (hfinfo->display != BASE_NONE) {
			tmp_str = val_to_str_wmem(NULL, hfinfo->display, hf_display, "(Bit count: %d)");
			g_error("Field '%s' (%s) is an %s but is being displayed as %s instead of BASE_NONE\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type),
				tmp_str);
			wmem_free(NULL, tmp_str);
		}
		if (hfinfo->bitmask != 0)
			g_error("Field '%s' (%s) is an %s but has a bitmask\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		if (hfinfo->strings != NULL)
			g_error("Field '%s' (%s) is an %s but has a strings value\n",
				hfinfo->name, hfinfo->abbrev,
				ftype_name(hfinfo->type));
		break;
	}
}
#define PRE_ALLOC_EXPERT_FIELDS_MEM 5000
/** Same as DISSECTOR_ASSERT(), but will throw DissectorError exception
* unconditionally, much like GLIB's g_assert_not_reached works.
*
* NOTE: this should only be used to detect bugs in the dissector (e.g., logic
* conditions that shouldn't happen).  It should NOT be used for showing
* that a packet is malformed.  For that, use expert_infos instead.
*
*/
#define DISSECTOR_ASSERT_NOT_REACHED()  \
  (REPORT_DISSECTOR_BUG("%s:%u: failed assertion \"DISSECTOR_ASSERT_NOT_REACHED\"", \
        __FILE__, __LINE__))
static header_field_info *same_name_hfinfo;
/* Hash table of abbreviations and IDs */
static GHashTable *gpa_name_map = NULL;
#define PROTO_PRE_ALLOC_HF_FIELDS_MEM (195000+PRE_ALLOC_EXPERT_FIELDS_MEM)
static int
proto_register_field_init(header_field_info *hfinfo, const int parent)
{

	tmp_fld_check_assert(hfinfo);

	hfinfo->parent = parent;
	hfinfo->same_name_next = NULL;
	hfinfo->same_name_prev_id = -1;

	/* if we always add and never delete, then id == len - 1 is correct */
	if (gpa_hfinfo.len >= gpa_hfinfo.allocated_len) {
		if (!gpa_hfinfo.hfi) {
			gpa_hfinfo.allocated_len = PROTO_PRE_ALLOC_HF_FIELDS_MEM;
			gpa_hfinfo.hfi = (header_field_info **)g_malloc(sizeof(header_field_info *)*PROTO_PRE_ALLOC_HF_FIELDS_MEM);
		}
		else {
			gpa_hfinfo.allocated_len += 1000;
			gpa_hfinfo.hfi = (header_field_info **)g_realloc(gpa_hfinfo.hfi,
				sizeof(header_field_info *)*gpa_hfinfo.allocated_len);
			/*g_warning("gpa_hfinfo.allocated_len %u", gpa_hfinfo.allocated_len);*/
		}
	}
	gpa_hfinfo.hfi[gpa_hfinfo.len] = hfinfo;
	gpa_hfinfo.len++;
	hfinfo->id = gpa_hfinfo.len - 1;

	/* if we have real names, enter this field in the name tree */
	if ((hfinfo->name[0] != 0) && (hfinfo->abbrev[0] != 0)) {

		header_field_info *same_name_next_hfinfo;
		guchar c;

		/* Check that the filter name (abbreviation) is legal;
		* it must contain only alphanumerics, '-', "_", and ".". */
		c = proto_check_field_name(hfinfo->abbrev);
		if (c) {
			if (c == '.') {
				fprintf(stderr, "Invalid leading, duplicated or trailing '.' found in filter name '%s'\n", hfinfo->abbrev);
			}
			else if (g_ascii_isprint(c)) {
				fprintf(stderr, "Invalid character '%c' in filter name '%s'\n", c, hfinfo->abbrev);
			}
			else {
				fprintf(stderr, "Invalid byte \\%03o in filter name '%s'\n", c, hfinfo->abbrev);
			}
			DISSECTOR_ASSERT_NOT_REACHED();
		}

		/* We allow multiple hfinfo's to be registered under the same
		* abbreviation. This was done for X.25, as, depending
		* on whether it's modulo-8 or modulo-128 operation,
		* some bitfield fields may be in different bits of
		* a byte, and we want to be able to refer to that field
		* with one name regardless of whether the packets
		* are modulo-8 or modulo-128 packets. */

		same_name_hfinfo = NULL;

		g_hash_table_insert(gpa_name_map, (gpointer)(hfinfo->abbrev), hfinfo);
		/* GLIB 2.x - if it is already present
		* the previous hfinfo with the same name is saved
		* to same_name_hfinfo by value destroy callback */
		if (same_name_hfinfo) {
			/* There's already a field with this name.
			* Put the current field *before* that field
			* in the list of fields with this name, Thus,
			* we end up with an effectively
			* doubly-linked-list of same-named hfinfo's,
			* with the head of the list (stored in the
			* hash) being the last seen hfinfo.
			*/
			same_name_next_hfinfo =
				same_name_hfinfo->same_name_next;

			hfinfo->same_name_next = same_name_next_hfinfo;
			if (same_name_next_hfinfo)
				same_name_next_hfinfo->same_name_prev_id = hfinfo->id;

			same_name_hfinfo->same_name_next = hfinfo;
			hfinfo->same_name_prev_id = same_name_hfinfo->id;
#ifdef ENABLE_CHECK_FILTER
			while (same_name_hfinfo) {
				if (_ftype_common(hfinfo->type) != _ftype_common(same_name_hfinfo->type))
					fprintf(stderr, "'%s' exists multiple times with NOT compatible types: %s and %s\n", hfinfo->abbrev, ftype_name(hfinfo->type), ftype_name(same_name_hfinfo->type));
				same_name_hfinfo = same_name_hfinfo->same_name_next;
			}
#endif
		}
	}

	return hfinfo->id;
}

int
proto_register_protocol(const char *name, const char *short_name,
	const char *filter_name)
{
	protocol_t *protocol;
	header_field_info *hfinfo;

	/*
	* Make sure there's not already a protocol with any of those
	* names.  Crash if there is, as that's an error in the code
	* or an inappropriate plugin.
	* This situation has to be fixed to not register more than one
	* protocol with the same name.
	*/

	if (g_hash_table_lookup(proto_names, name)) {
		/* g_error will terminate the program */
		g_error("Duplicate protocol name \"%s\"!"
			" This might be caused by an inappropriate plugin or a development error.", name);
	}

	if (g_hash_table_lookup(proto_short_names, short_name)) {
		g_error("Duplicate protocol short_name \"%s\"!"
			" This might be caused by an inappropriate plugin or a development error.", short_name);
	}

	check_valid_filter_name_or_fail(filter_name);

	if (g_hash_table_lookup(proto_filter_names, filter_name)) {
		g_error("Duplicate protocol filter_name \"%s\"!"
			" This might be caused by an inappropriate plugin or a development error.", filter_name);
	}

	/*
	* Add this protocol to the list of known protocols;
	* the list is sorted by protocol short name.
	*/
	protocol = g_new(protocol_t, 1);
	protocol->name = name;
	protocol->short_name = short_name;
	protocol->filter_name = filter_name;
	protocol->fields = NULL; /* Delegate until actually needed */
	protocol->is_enabled = TRUE; /* protocol is enabled by default */
	protocol->enabled_by_default = TRUE; /* see previous comment */
	protocol->can_toggle = TRUE;
	protocol->parent_proto_id = -1;
	protocol->heur_list = NULL;

	/* List will be sorted later by name, when all protocols completed registering */
	protocols = g_list_prepend(protocols, protocol);
	g_hash_table_insert(proto_names, (gpointer)name, protocol);
	g_hash_table_insert(proto_filter_names, (gpointer)filter_name, protocol);
	g_hash_table_insert(proto_short_names, (gpointer)short_name, protocol);

	/* Here we allocate a new header_field_info struct */
	hfinfo = g_slice_new(header_field_info);
	hfinfo->name = name;
	hfinfo->abbrev = filter_name;
	hfinfo->type = FT_PROTOCOL;
	hfinfo->display = BASE_NONE;
	hfinfo->strings = protocol;
	hfinfo->bitmask = 0;
	hfinfo->ref_type = HF_REF_TYPE_NONE;
	hfinfo->blurb = NULL;
	hfinfo->parent = -1; /* This field differentiates protos and fields */

	protocol->proto_id = proto_register_field_init(hfinfo, hfinfo->parent);
	return protocol->proto_id;
}
static expert_level_entry_t *uat_expert_entries = NULL;
typedef struct
{
	char    *field;
	guint32  severity;
} expert_level_entry_t;
const value_string expert_severity_vals[] = {
	{ PI_ERROR,             "Error" },
	{ PI_WARN,              "Warning" },
	{ PI_NOTE,              "Note" },
	{ PI_CHAT,              "Chat" },
	{ PI_COMMENT,           "Comment" },
	{ 1,                    "Ok" },
	{ 0, NULL }
};
protocol_t *
find_protocol_by_id(const int proto_id)
{
	header_field_info *hfinfo;

	if (proto_id < 0)
		return NULL;

	PROTO_REGISTRAR_GET_NTH(proto_id, hfinfo);
	if (hfinfo->type != FT_PROTOCOL) {
		DISSECTOR_ASSERT(hfinfo->display & BASE_PROTOCOL_INFO);
	}
	return (protocol_t *)hfinfo->strings;
}
static int
proto_register_field_common(protocol_t *proto, header_field_info *hfi, const int parent)
{
	if (proto != NULL) {
		g_ptr_array_add(proto->fields, hfi);
	}

	return proto_register_field_init(hfi, parent);
}
/* for use with static arrays only, since we don't allocate our own copies
of the header_field_info struct contained within the hf_register_info struct */
void
proto_register_field_array(const int parent, hf_register_info *hf, const int num_records)
{
	hf_register_info *ptr = hf;
	protocol_t	 *proto;
	int		  i;

	proto = find_protocol_by_id(parent);

	if (proto->fields == NULL) {
		proto->fields = g_ptr_array_sized_new(num_records);
	}

	for (i = 0; i < num_records; i++, ptr++) {
		/*
		* Make sure we haven't registered this yet.
		* Most fields have variables associated with them
		* that are initialized to -1; some have array elements,
		* or possibly uninitialized variables, so we also allow
		* 0 (which is unlikely to be the field ID we get back
		* from "proto_register_field_init()").
		*/
		if (*ptr->p_id != -1 && *ptr->p_id != 0) {
			fprintf(stderr,
				"Duplicate field detected in call to proto_register_field_array: %s is already registered\n",
				ptr->hfinfo.abbrev);
			return;
		}

		*ptr->p_id = proto_register_field_common(proto, &ptr->hfinfo, parent);
	}
}
/* Useful when you have an array whose size you can tell at compile-time */
#define array_length(x)	(sizeof x / sizeof x[0])
int		num_tree_types;

/* Points to the first element of an array of bits, indexed by
a subtree item type; that array element is TRUE if subtrees of
an item of that type are to be expanded. */
static guint32 *tree_is_expanded;
void
proto_register_subtree_array(gint *const *indices, const int num_indices)
{
	int	i;
	gint	*const *ptr = indices;

	/*
	* If we've already allocated the array of tree types, expand
	* it; this lets plugins such as mate add tree types after
	* the initial startup.  (If we haven't already allocated it,
	* we don't allocate it; on the first pass, we just assign
	* ett values and keep track of how many we've assigned, and
	* when we're finished registering all dissectors we allocate
	* the array, so that we do only one allocation rather than
	* wasting CPU time and memory by growing the array for each
	* dissector that registers ett values.)
	*/
	if (tree_is_expanded != NULL) {
		tree_is_expanded = (guint32 *)g_realloc(tree_is_expanded, (1 + ((num_tree_types + num_indices) / 32)) * sizeof(guint32));

		/* set new items to 0 */
		/* XXX, slow!!! optimize when needed (align 'i' to 32, and set rest of guint32 to 0) */
		for (i = num_tree_types; i < num_tree_types + num_indices; i++)
			tree_is_expanded[i >> 5] &= ~(1U << (i & 31));
	}

	/*
	* Assign "num_indices" subtree numbers starting at "num_tree_types",
	* returning the indices through the pointers in the array whose
	* first element is pointed to by "indices", and update
	* "num_tree_types" appropriately.
	*/
	for (i = 0; i < num_indices; i++, ptr++, num_tree_types++) {
		if (**ptr != -1) {
			/* g_error will terminate the program */
			g_error("register_subtree_array: subtree item type (ett_...) not -1 !"
				" This is a development error:"
				" Either the subtree item type has already been assigned or"
				" was not initialized to -1.");
		}
		**ptr = num_tree_types;
	}
}
void
proto_set_cant_toggle(const int proto_id)
{
	protocol_t *protocol;

	protocol = find_protocol_by_id(proto_id);
	protocol->can_toggle = FALSE;
}
module_t *
prefs_register_protocol(int id, void(*apply_cb)(void))
{
	protocol_t *protocol;

	/*
	* Have we yet created the "Protocols" subtree?
	*/
	if (protocols_module == NULL) {
		/*
		* No.  Register Protocols subtree as well as any preferences
		* for non-dissector modules.
		*/
		pre_init_prefs();
		prefs_register_modules();
	}
	protocol = find_protocol_by_id(id);
	if (protocol == NULL)
		g_error("Protocol preferences being registered with an invalid protocol ID");
	return prefs_register_module(protocols_module,
		proto_get_protocol_filter_name(id),
		proto_get_protocol_short_name(protocol),
		proto_get_protocol_name(id), apply_cb, TRUE);
}
void
expert_packet_init(void)
{
	module_t *module_expert;
	uat_t    *expert_uat;

	static hf_register_info hf[] = {
		{ &hf_expert_msg,
	{ "Message", "_ws.expert.message", FT_STRING, BASE_NONE, NULL, 0, "Wireshark expert information", HFILL }
		},
	{ &hf_expert_group,
	{ "Group", "_ws.expert.group", FT_UINT32, BASE_NONE, VALS(expert_group_vals), 0, "Wireshark expert group", HFILL }
	},
	{ &hf_expert_severity,
	{ "Severity level", "_ws.expert.severity", FT_UINT32, BASE_NONE, VALS(expert_severity_vals), 0, "Wireshark expert severity level", HFILL }
	}
	};
	static gint *ett[] = {
		&ett_expert,
		&ett_subexpert
	};

	/* UAT for overriding severity levels */
	static uat_field_t custom_expert_fields[] = {
		UAT_FLD_CSTRING(uat_expert_entries, field, "Field name", "Expert Info filter name"),
		UAT_FLD_VS(uat_expert_entries, severity, "Severity", expert_severity_vals, "Custom severity level"),
		UAT_END_FIELDS
	};

	if (expert_tap == -1) {
		expert_tap = register_tap("expert");
	}

	if (proto_expert == -1) {
		proto_expert = proto_register_protocol("Expert Info", "Expert", "_ws.expert");
		proto_register_field_array(proto_expert, hf, array_length(hf));
		proto_register_subtree_array(ett, array_length(ett));
		proto_set_cant_toggle(proto_expert);

		module_expert = prefs_register_protocol(proto_expert, NULL);

		expert_uat = uat_new("Expert Info Severity Level Configuration",
			sizeof(expert_level_entry_t),
			"expert_severity",
			TRUE,
			(void **)&uat_expert_entries,
			&expert_level_entry_count,
			UAT_AFFECTS_DISSECTION,
			NULL,
			uat_expert_copy_cb,
			uat_expert_update_cb,
			uat_expert_free_cb,
			uat_expert_post_update_cb,
			NULL,
			custom_expert_fields);

		prefs_register_uat_preference(module_expert,
			"expert_severity_levels",
			"Severity Level Configuration",
			"A table that overrides Expert Info field severity levels to user configured levels",
			expert_uat);

	}

	highest_severity = 0;

	proto_malformed = proto_get_id_by_filter_name("_ws.malformed");
}

void
init_dissection(void)
{
	/*
	* Reinitialize resolution information. Don't leak host entries from
	* one file to another (e.g. embarassing-host-name.example.com from
	* file1.pcapng into a name resolution block in file2.pcapng).
	*/
	host_name_lookup_reset();

	wmem_enter_file_scope();

	/* Initialize the table of conversations. */
	epan_conversation_init();

	/* Initialize protocol-specific variables. */
	g_slist_foreach(init_routines, &call_routine, NULL);

	/* Initialize the stream-handling tables */
	stream_init();

	/* Initialize the expert infos */
	expert_packet_init();
}
epan_t *
epan_new(struct packet_provider_data *prov,
	const struct packet_provider_funcs *funcs)
{
	epan_t *session = g_slice_new0(epan_t);

	session->prov = prov;
	session->funcs = *funcs;

	/* XXX, it should take session as param */
	init_dissection();

	return session;
}
static epan_t *
ws_epan_new(capture_file *cf)
{
	static const struct packet_provider_funcs funcs = {
		ws_get_frame_ts,
		cap_file_provider_get_interface_name,
		cap_file_provider_get_interface_description,
		cap_file_provider_get_user_comment
	};

	return epan_new(&cf->provider, &funcs);
}


void wtap_set_cb_new_ipv4(wtap *wth, wtap_new_ipv4_callback_t add_new_ipv4) {
	if (wth)
		wth->add_new_ipv4 = add_new_ipv4;
}

void wtap_set_cb_new_ipv6(wtap *wth, wtap_new_ipv6_callback_t add_new_ipv6) {
	if (wth)
		wth->add_new_ipv6 = add_new_ipv6;
}
// If we ever add the ability to open multiple capture files we might be
// able to use something like QMap<capture_file *, PacketList *> to match
// capture files against packet lists and models.
static PacketList *gbl_cur_packet_list = NULL;

/* Redraw the packet list *and* currently-selected detail */
void
packet_list_queue_draw(void)
{
	if (gbl_cur_packet_list)
		gbl_cur_packet_list->redrawVisiblePackets();
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