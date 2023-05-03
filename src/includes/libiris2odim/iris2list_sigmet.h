/* *********************************************************************
 * *                                                                   *
 * *     Sigmet-IRIS Internal Definitions                              *
 * *                                                                   *
 * *********************************************************************
 * File: iris2list_sigmet.h
 *
 *  COPYRIGHT (c) 1991, 1992, 1994, 1996, 1997, 1999, 2001, 2002, 2003,
 *                         2004, 2009, 2011 BY
 *             VAISALA INC., WESTFORD MASSACHUSETTS, U.S.A.
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED.
 */
/**
 * Description: These structures and macro definitions were copied from
 * various header files used in sigmet-IRIS radar data processing software.  
 * These are used in iris2list.c to read "IRIS RAW" radar data files. Part
 * of program iris2odim.
 * @file iris2list_sigmet.h
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-11-25
 */
#ifndef IRIS2LIST_SIGMET_H
#define IRIS2LIST_SIGMET_H
#include <stdint.h>

/* some macros used for byte swapping if input file is big endian */
/* Swap 2 bytes, unsigned 16 bit values: */
#define Swap2Bytes(val) \
( (((val) >> 8) & 0x00FF) | (((val) << 8) & 0xFF00) )

/* Swap 4 byte, 32 bit values: */
#define Swap4Bytes(val) \
 ( (((val) >> 24) & 0x000000FF) | (((val) >>  8) & 0x0000FF00) | \
   (((val) <<  8) & 0x00FF0000) | (((val) << 24) & 0xFF000000) )

/* Swap 8 byte, 64 bit values: */
#define Swap8Bytes(val) \
 ( (((val) >> 56) & 0x00000000000000FF) | (((val) >> 40) & 0x000000000000FF00) | \
   (((val) >> 24) & 0x0000000000FF0000) | (((val) >>  8) & 0x00000000FF000000) | \
   (((val) <<  8) & 0x000000FF00000000) | (((val) << 24) & 0x0000FF0000000000) | \
   (((val) << 40) & 0x00FF000000000000) | (((val) << 56) & 0xFF00000000000000) )


/* use some typedefs to produce shorter declaration statements */
typedef int8_t          SINT1;          /* signed 8-bit integer */
typedef uint8_t         UINT1;          /* unsigned 8-bit integer */
typedef int16_t         SINT2;          /* signed 16-bit integer */
typedef uint16_t        UINT2;          /* unsigned 16-bit integer */
typedef int32_t         SINT4;          /* signed 32-bit integer */
typedef uint32_t        UINT4;          /* unsigned 32-bit integer */
typedef int64_t         SINT8;          /* signed 64-bit integer */
typedef uint64_t        UINT8;          /* unsigned 64-bit integer */
typedef float           FLT4;           /* floating-point number */
typedef double          FLT8;           /* double-precision floating-point number */
typedef int16_t         BIN2;           /* 16-bit binary angle */
typedef uint32_t        BIN4;           /* 32-bit binary angle */
typedef uint32_t        MESSAGE;        /* encoded error value */


#ifndef FALSE
#define TRUE  (1)
#define FALSE (0)
#endif




#define COMPRESSION_REQUIRED 0
#define CHECKSUM_REQUIRED 0
#define MY_NC_DEFLATE 1
#define MY_NC_DEFLATE_LEVEL 9

#define MAX_DATA_TYPES_IN_FILE 21      /*Max # data types in a single IRIS RAW file*/
#define MAX_SWEEPS 40                  /*Max # sweeps in any volume-scan / scan-mode */
#define DEFAULT_BYTES_IN_STRING 80
// define structure sizes in bytes
#define RAY_HSIZE (12)
#define STRUCT_HEADER_SIZE   (12)
#define YMDS_TIME_SIZE   (12)
#define RAW_PROD_BHDR_SIZE  (12)
#define PSI_SIZE (80)
#define PRODUCT_END_SIZE  (308)
#define PRODUCT_CONFIGURATION_SIZE  (320)
#define INGEST_CONFIGURATION_SIZE (480)
#define TASK_CONFIGURATION_SIZE (2612)
#define DSP_GPARM_SIZE (128)
#define PRODUCT_HDR_SIZE (STRUCT_HEADER_SIZE + \
                  PRODUCT_CONFIGURATION_SIZE + \
                  PRODUCT_END_SIZE)
#define INGEST_HEADER_SIZE ( STRUCT_HEADER_SIZE + \
    INGEST_CONFIGURATION_SIZE + TASK_CONFIG_SIZE + \
    732 + DSP_GPARM_SIZE + 920 )
#define INGEST_DATA_HEADER_SIZE  76
#define TASK_PSCAN_INFO_SIZE (200)
#define TASK_SCHED_INFO_SIZE  120
#define TASK_DSP_MODE_SIZE  32
#define TASK_DSP_INFO_SIZE  320
#define TASK_CALIB_INFO_SIZE  320
#define TASK_RANGE_INFO_SIZE  160
#define TASK_SCAN_INFO_SIZE  320
#define TASK_MISC_INFO_SIZE  320
#define TASK_CONF_END_SIZE  320          /* Size of struct's end area: */
#define TASK_COMNT_SIZE     720          /* Size of Comment buffer */
#define TASK_CONFIG_SIZE ( STRUCT_HEADER_SIZE + TASK_SCHED_INFO_SIZE + \
   TASK_DSP_INFO_SIZE  + TASK_CALIB_INFO_SIZE + TASK_RANGE_INFO_SIZE + \
   TASK_SCAN_INFO_SIZE + TASK_MISC_INFO_SIZE  + TASK_CONF_END_SIZE   + \
   TASK_COMNT_SIZE )

// define ray/input-record/array sizes in bytes
#define IRIS_BUFFER_SIZE (6144)
#define MAX_RAY_BODY_SIZE (IRIS_BUFFER_SIZE-RAY_HSIZE)

#define YMDS_FLG_DST  (0x0400)   /* Time is in Daylight Savings Time */
#define YMDS_FLG_UTC  (0x0800)   /* Time is UTC (else local) */
#define YMDS_FLG_LDST (0x1000)   /* Local time is in Daylight Savings Time */
/* Extract the milliseconds from the ymds structure */
#define YMDS_MASK_FLAGS   (0xfc00)
#define YMDS_MASK_MS      (0x03ff)
#define DST_FROM_MILLS( _MILLS )   (0 != ( YMDS_FLG_DST & (_MILLS) ))
#define UTC_FROM_MILLS( _MILLS )   (0 != ( YMDS_FLG_UTC & (_MILLS) ))
#define LDST_FROM_MILLS( _MILLS )  (0 != ( YMDS_FLG_LDST & (_MILLS) ))
#define FLAGS_FROM_MILLS( _MILLS ) ( YMDS_MASK_FLAGS & (_MILLS) )
#define MS_FROM_MILLS( _MILLS )    ( YMDS_MASK_MS & (_MILLS) )
#define MILLS_FROM_MS_FLAGS( _MS, _FLAGS ) ((YMDS_MASK_MS&(_MS))|(YMDS_MASK_FLAGS&(_FLAGS)))
#define TRACK_FLG_DIAGNOSTIC (0x00000200)
#define TRACK_FLGS_ALL (TRACK_FLG_DIAGNOSTIC)
#define THICK_FLG_PSUEDO   (0x0001) /* Fake it if top or bottom missing */
#define THICK_FLGS_ALL (THICK_FLG_PSUEDO)  /* Used in the THICK PCO */
#define IRAW_CONVERT_PRESERVE 0      /* Preserve all INGEST data */
#define IRAW_CONVERT_8_TO_16  1      /* Convert 8-bit data to 16-bit data */
#define IRAW_CONVERT_16_TO_8  2      /* Convert 16-bit data to 8-bit data */
#define TASK_MSC_DSSIM       0x0001     /* Digital Signal Simulator in use */
#define TASK_MSC_PARTIAL     0x0002     /* Volume scan was halted prematurely */
#define TASK_MSC_KEEP        0x0010     /* Keep this file (WATCHDOG info) */
#define TASK_MSC_CLUTMAP     0x0020     /* File tagged as a clutter map (INGEST info) */
/* What kind of polarization transmission is being used.  These
 * bits are input to the PROC command, and antenna library where
 * relevant.
 */
#define POL_HORIZ_FIX    (0)    /* Horizontal fixed */
#define POL_VERT_FIX     (1)    /* Vertical fixed */
#define POL_ALTERNATING  (2)    /* Alternating between H and V */
#define POL_SIMULTANEOUS (3)    /* Simultaneous on both H and V */
#define POL_UNCHANGED    (7)    /* Do not change the polarization (in RCP02) */
/* Options for transmitted phase sequence.
 */
#define PHSEQ_FIXED   (0)       /* Unmodulated output */
#define PHSEQ_RANDOM  (1)       /* Random sequence */
#define PHSEQ_CUSTOM  (2)       /* Custom user defined sequence */
#define PHSEQ_SZ8_64  (3)       /* Sachi/Zrnic sequence */
/*                                                                         *
 * Handle NetCDF errors by printing an error message and exiting with a    *
 * non-zero status.                                                        */
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(-1);}
/*
 * Macros for general use
 */
#define BIT( ibit ) ( 1 << (ibit) )
#define BTEST( ivalue, ibit ) (((ivalue) &  BIT(ibit)) != 0 )
#define IBSET( ivalue, ibit ) ( (ivalue) |  BIT(ibit) )
#define IBCLR( ivalue, ibit ) ( (ivalue) & ~BIT(ibit) )
#define IBINV( ivalue, ibit ) ( (ivalue) ^  BIT(ibit) )
/*****************************************************************************
*                                                                            *
*  ------------------- Data Parameter Type Definitions --------------------  *
*                                                                            *
*****************************************************************************/
/* These are the bit numbers that are used in many places to specify a
 * choice of data type and to tell what kind of data to process.  Note
 * that the "Extended Header" type is included here, though it is not
 * generated by the DSP.  Historically, types 0-31 were directly
 * produced by the DSP, but that distinction is no longer maintained.
 */
enum iris_data_type
{
    DB_XHDR          =  0,  /* Extended Headers  */
    DB_DBT           =  1,  /* Total H power (1 byte) */
    DB_DBZ           =  2,  /* Clutter Corrected H reflectivity (1 byte) */
    DB_VEL           =  3,  /* Velocity (1 byte) */
    DB_WIDTH         =  4,  /* Width (1 byte) */
    DB_ZDR           =  5,  /* Differential reflectivity (1 byte) */
    DB_ORAIN         =  6,  /* Old Rainfall rate (stored as dBZ), not used */
    DB_DBZC          =  7,  /* Fully corrected reflectivity (1 byte) */
    DB_DBT2          =  8,  /* Uncorrected reflectivity (2 byte) */
    DB_DBZ2          =  9,  /* Corrected reflectivity (2 byte) */
    DB_VEL2          = 10,  /* Velocity (2 byte) */
    DB_WIDTH2        = 11,  /* Width (2 byte) */
    DB_ZDR2          = 12,  /* Differential reflectivity (2 byte) */
    DB_RAINRATE2     = 13,  /* Rainfall rate (2 byte) */
    DB_KDP           = 14,  /* Kdp (specific differential phase)(1 byte) */
    DB_KDP2          = 15,  /* Kdp (specific differential phase)(2 byte) */
    DB_PHIDP         = 16,  /* PHIdp (differential phase)(1 byte) */
    DB_VELC          = 17,  /* Corrected Velocity (1 byte) */
    DB_SQI           = 18,  /* SQI (1 byte) */
    DB_RHOHV         = 19,  /* RhoHV(0) (1 byte) */
    DB_RHOHV2        = 20,  /* RhoHV(0) (2 byte) */
    DB_DBZC2         = 21,  /* Fully corrected reflectivity (2 byte) */
    DB_VELC2         = 22,  /* Corrected Velocity (2 byte) */
    DB_SQI2          = 23,  /* SQI (2 byte) */
    DB_PHIDP2        = 24,  /* PHIdp (differential phase)(2 byte) */
    DB_LDRH          = 25,  /* LDR H to V (1 byte) */
    DB_LDRH2         = 26,  /* LDR H to V (2 byte) */
    DB_LDRV          = 27,  /* LDR V to H (1 byte) */
    DB_LDRV2         = 28,  /* LDR V to H (2 byte) */
    DB_FLAGS         = 29,  /* Individual flag bits for each bin */
    DB_FLAGS2        = 30,  /*  (See bit definitions below) */
    DB_FLOAT32       = 31,  /* Test of floating format */
    DB_HEIGHT        = 32,  /* Height (1/10 km) (1 byte) */
    DB_VIL2          = 33,  /* Linear liquid (.001mm) (2 byte) */
    DB_NULL          = 34,  /* Data type is not applicable  */
    DB_SHEAR         = 35,  /* Wind Shear (1 byte) */
    DB_DIVERGE2      = 36,  /* Divergence (.001 10**-4) (2-byte) */
    DB_FLIQUID2      = 37,  /* Floated liquid (2 byte) */
    DB_USER          = 38,  /* User type, unspecified data (1 byte) */
    DB_OTHER         = 39,  /* Unspecified data, no color legend */
    DB_DEFORM2       = 40,  /* Deformation (.001 10**-4) (2-byte) */
    DB_VVEL2         = 41,  /* Vertical velocity (.01 m/s) (2-byte) */
    DB_HVEL2         = 42,  /* Horizontal velocity (.01 m/s) (2-byte) */
    DB_HDIR2         = 43,  /* Horizontal wind direction (.1 degree) (2-byte) */
    DB_AXDIL2        = 44,  /* Axis of Dillitation (.1 degree) (2-byte) */
    DB_TIME2         = 45,  /* Time of data (seconds) (2-byte) */
    DB_RHOH          = 46,  /* Rho H to V (1 byte) */
    DB_RHOH2         = 47,  /* Rho H to V (2 byte) */
    DB_RHOV          = 48,  /* Rho V to H (1 byte) */
    DB_RHOV2         = 49,  /* Rho V to H (2 byte) */
    DB_PHIH          = 50,  /* Phi H to V (1 byte) */
    DB_PHIH2         = 51,  /* Phi H to V (2 byte) */
    DB_PHIV          = 52,  /* Phi V to H (1 byte) */
    DB_PHIV2         = 53,  /* Phi V to H (2 byte) */
    DB_USER2         = 54,  /* User type, unspecified data (2 byte) */
    DB_HCLASS        = 55,  /* Hydrometeor class (1 byte) */
    DB_HCLASS2       = 56,  /* Hydrometeor class (2 byte) */
    DB_ZDRC          = 57,  /* Corrected Differential reflectivity (1 byte) */
    DB_ZDRC2         = 58,  /* Corrected Differential reflectivity (2 byte) */
    DB_TEMPERATURE16 = 59,  /* Temperature (2 byte) */
    DB_VIR16         = 60,  /* Vertically Integrated Reflectivity (2 byte) */
    DB_DBTV8         = 61,  /* Total V Power (1 byte) */
    DB_DBTV16        = 62,  /* Total V Power (2 byte) */
    DB_DBZV8         = 63,  /* Clutter Corrected V Reflectivity (1 byte) */
    DB_DBZV16        = 64,  /* Clutter Corrected V Reflectivity (2 byte) */
    DB_SNR8          = 65,  /* Signal to Noise ratio (1 byte) */
    DB_SNR16         = 66,  /* Signal to Noise ratio (2 byte) */
    DB_ALBEDO8       = 67,  /* Albedo (1 byte) */
    DB_ALBEDO16      = 68,  /* Albedo (2 byte) */
    DB_VILD16        = 69,  /* VIL Density (2 byte) */
    DB_TURB16        = 70,  /* Turbulence (2 byte) */
    DB_DBTE8         = 71,  /* Total Power Enhanced (via H+V or HV) (1 byte) */
    DB_DBTE16        = 72,  /* Total Power Enhanced (via H+V or HV) (2 byte) */
    DB_DBZE8         = 73,  /* Clutter Corrected Reflectivity Enhanced (1 byte) */
    DB_DBZE16        = 74,  /* Clutter Corrected Reflectivity Enhanced (2 byte) */

    NUM_DEFINED_DATA = 75      /* Total number of defined data types */
};

typedef char *astring;

// define various enums
enum product_type_values {
  PPI_type=1, RHI_type=2, CAPPI_type=3, CROSS_type=4,
  TOPS_type=5, TRACK_type=6, RAIN1_type=7, RAINN_type=8,
  VVP_type=9, VIL_type=10, SHEAR_type=11, WARN_type=12,
  CATCH_type=13, RTI_type=14, RAW_type=15, MAX_type=16,
  USER_type=17, USERV_type=18, OTHER_type=19, STATUS_type=20,
  SLINE_type=21,WIND_type=22, BEAM_type=23, TEXT_type=24,
  FCAST_type=25, NDOP_type=26, IMAGE_type=27, COMP_type=28,
  TDWR_type=29, GAGE_type=30, DWELL_type=31, SRI_type=32,
  BASE_type=33, HMAX_type=34, VAD_type=35, THICK_type=36,
  SATELLITE_Type=37, LAYER_type=38};

enum struct_id_values {
  Task_configuration_id    = 22, Ingest_header_id  = 23,
  Ingest_data_header_id    = 24, Tape_inventory_id = 25,
  Product_configuration_id = 26, Product_hdr_id    = 27,
  Tape_header_record_id    = 28 };

enum scheduling_code_values {
  sched_hold=0, sched_run_next=1, sched_all=2,
  sched_run_again, sched_run_once };

enum projection_type_values {
  Azimuthal_equidistant = 0, Mercator = 1, Polar_stereographic = 2,
  UTM = 3, Perspective_from_geosync = 4, Equidistant_cylindrical = 5,
  Gnonomic = 6, Gauss_Conformal = 7, Lambert_Conforma_Conic = 8 };
  
// define structure 'ymds_time' and type 'ymd_s'
typedef struct ymds_time {
  UINT4 seconds_since_midnight;
  UINT2 milliseconds_and_UTC_DST_indication;
  // milliseconds in lower 10 bits
  // bit 10: time is daylight savings time
  // bit 11: time is UTC
  // bit 12: local time is daylight savings time
  UINT2 year;
  UINT2 month;
  UINT2 day;
} ymd_s;
  
// define structure 'beam_psi_struct' and type 'beam_psi_struct'
typedef struct beam_psi_struct {
  UINT4 minimum_range_in_cm;
  UINT4 maximum_range_in_cm;
  BIN4 left_azimuth;
  BIN4 right_azimuth;
  BIN4 lower_elevation;
  BIN4 upper_elevation;
  BIN4 azimuth_smoothing;
  BIN4 elevation_smoothing;
  BIN4 azimuth_of_sun_at_start;
  BIN4 elevation_of_sun_at_start;
  BIN4 azimuth_of_sun_at_end;
  BIN4 elevation_of_sun_at_end;
  char ipad_end[PSI_SIZE-48];
} beam_psi_struct;

// define structure 'catch_psi_struct' and type 'catch_psi_struct'
typedef struct catch_psi_struct {
  UINT4 flags;
#define CATCH_FLG_WARNINGS_ON (0x0001)
  UINT4 hours_of_accumulation;
  SINT4 threshold_offset_in_thousandths_or_mm;
  SINT4 threshold_fraction_in_thousandths;
  char name_of_RAIN1_product[12];
  char name_of_catchment_file_to_use[16];
  UINT4 seconds_of_accumulation; // in low 16 bits
  UINT4 min_Z_RAIN1;
  UINT4 span_in_seconds_RAIN1;
  UINT4 ave_Gage_correction_factor; // in low 16 bits
  char ipad_end[PSI_SIZE-60];
} catch_psi_struct;

// define structure 'cappi_psi_struct' and type 'cappi_psi_struct'
typedef struct cappi_psi_struct {
  UINT4 shear_flags;
  SINT4 cappi_height_cm_above_ref;
  UINT2 flags; // bit0 = make pseudo CAPPI, bit1 = velocity is horizontal winds
#define CAPPI_FLG_PSEUDO (0x0001)   /* Make a pseudo CAPPI (extend single tilts) */
#define CAPPI_FLG_HORVEL (0x0002)   /* Modify velocity to be horizontal winds */
#define CAPPI_FLGS_ALL (CAPPI_FLG_PSEUDO|CAPPI_FLG_HORVEL)
  BIN2 azimuth_smoothing_for_shear;
  char shear_correction_name[12];
  UINT4 max_age_of_shear_correction_in_seconds;
  char ipad_end[PSI_SIZE-28];
} cappi_psi_struct;

// define structure 'cross_psi_struct' and type 'cross_psi_struct'
typedef struct cross_psi_struct {
  BIN2 azimuth_angle_of_left2right_line_on_picture; // azimuths are defined clockwise from North
  UINT2 flags; // Bit0 = Input is cube, not polar.
#define CROSS_FLG_CUBE_IN (0x01) /* Input data is a data-cube, not polar */
  char eight_spare_bytes[8];
  SINT4 east_coord_of_center_in_cm; // relative to radar
  SINT4 north_coord_of_center_in_cm; // relative to radar
  char  name_of_data_cube_file[12]; // Name of data-cube file, space padded
  SINT4 user_miscellaneous[(PSI_SIZE-32)/4]; // unspecified contents
} cross_psi_struct;

// define structure 'fcast_psi_struct' and type 'fcast_psi_struct'
typedef struct fcast_psi_struct {
  UINT4 correlation_threshold; // 0-100
  SINT4 data_threshold;
  SINT4 mean_speed_in_cm_per_hour; // zero if none
  BIN4 direction_of_mean_speed;
  UINT4 maximum_time_between_products_in_seconds;
  SINT4 maximum_allowable_velocity_in_cm_per_seconds;
  UINT4 flags;
  SINT4 desired_output_resolution_in_cm;
  UINT4 type_of_input_product;
  char name_of_input_product[12]; // space padded
  char ipad_end[PSI_SIZE-48];
} fcast_psi_struct;

// define structure 'maximum_psi_struct' and type 'maximum_psi_struct'
typedef struct maximum_psi_struct {
  SINT1 four_spare_bytes[4];
  SINT4 bottom_of_interval_in_cm;
  SINT4 top_of_interval_in_cm;
  SINT4 number_of_pixels_in_side_panels;
  SINT2 horizontal_smoother_in_side_panels;
  SINT2 vertical_smoother_in_side_panels;
  char ipad_end[PSI_SIZE-20];
} maximum_psi_struct;

// define structure 'ppi_psi_struct' and type 'ppi_psi_struct'
typedef struct ppi_psi_struct {
  BIN2 elevation_angle;
  char two_spare_bytes[2];
  SINT4 max_range_in_cm; // zero=none
  SINT4 max_height_above_ref_in_cm; // zero=none
  char ipad_end[PSI_SIZE-12];
} ppi_psi_struct;

// define structure 'rain_psi_struct' and type 'rain_psi_struct'
typedef struct rain_psi_struct {
  UINT4 minimum_Z_to_accumulate;          // in 2-byte reflectivity format
  UINT2 average_gage_correction_factor;   // in 2-byte reflectivity format
  UINT2 seconds_of_accumulation;
  UINT2 flag_word; // Flag word: Bit0:  apply clutter map
                   //            Bit1:  apply gage correction
                   //            Bit2:  clutter map applied
                   //            Bit3:  gage correction was applied
#define RAIN_SPAN_MULTIPLE (900)  /* All rain product spans are a multiple of 15 minutes*/
#define RAINN_MAX_HOURS    (168)  /* Max number of hours in a rainn span */

#define RAIN_FLG_CLT_MAP    (0x0001) /* Use clutter map */
#define RAIN_FLG_CLT_MAP_OK (0x0004) /* Clutter map was actually used */
#define RAIN_FLG_GAG_COR    (0x0002) /* Apply raingage correction */
#define RAIN_FLG_GAG_COR_OK (0x0008) /* Raingage correction was actually used */
#define RAIN_FLG_DIAGNOSTIC (0x0010) /* Make diagnostic product */
#define RAIN_FLG_SRI_INPUT  (0x0020) /* Input is SRI type, else CAPPI */
#define RAIN1_FLGS_USED (RAIN_FLG_CLT_MAP|RAIN_FLG_GAG_COR|RAIN_FLG_DIAGNOSTIC|RAIN_FLG_SRI_INPUT)  /* Used in the RAIN1 PCO */
#define RAIN_FLG_USED RAIN1_FLGS_USED
#define RAINN_FLGS_KEPT (RAIN_FLG_CLT_MAP|RAIN_FLG_CLT_MAP_OK|RAIN_FLG_GAG_COR|RAIN_FLG_GAG_COR_OK|RAIN_FLG_DIAGNOSTIC|RAIN_FLG_SRI_INPUT)  /* Kept in the RAINN PCO */
  SINT2 number_of_hours_to_accumulate; // RAINN only
  char name_of_input_product_to_use[12]; // space padded
  UINT4 span_in_seconds_of_the_input_files;
  char ipad_end[PSI_SIZE-28];
} rain_psi_struct;

// define structure 'raw_psi_struct' and type 'raw_psi_struct'
typedef struct raw_psi_struct {
  UINT4 data_type_mask_word_0;
  SINT4 range_of_last_bin_in_cm;
  UINT4 format_conversion_flag; // 0 = preserve all ingest data
                                // 1 = convert 8-bit data to 16-bit data
                                // 2 = convert 16-bit data to 8-bit data
#define IRAW_CONVERT_PRESERVE 0      /* Preserve all INGEST data */
#define IRAW_CONVERT_8_TO_16  1      /* Convert 8-bit data to 16-bit data */
#define IRAW_CONVERT_16_TO_8  2      /* Convert 16-bit data to 8-bit data */
  UINT4 flag_word; // Bit0 = separate product files by sweep
                   // Bit1 = mask data by supplied mask
#define RAW_FLG_SWEEP    (0x00000001) /* Make sweep-by-sweep product files */
#define RAW_FLG_DATAMASK (0x00000002) /* Mask the data types recorded */
#define RAW_FLGS_ALL (RAW_FLG_SWEEP|RAW_FLG_DATAMASK) /* All bits in use */
  SINT4 sweep_number_if_separate_files; // origin 1
  UINT4 xhdr_type; //unused
  UINT4 data_type_mask_1;
  UINT4 data_type_mask_2;
  UINT4 data_type_mask_3;
  UINT4 data_type_mask_4;
  UINT4 playback_version; // low 16 bits
  char ipad_end[PSI_SIZE-44];
} raw_psi_struct;

// define structure 'rhi_psi_struct' and type 'rhi_psi_struct'
typedef struct rhi_psi_struct {
  BIN2 azimuth_angle;
  char ipad_end[PSI_SIZE-2];
} rhi_psi_struct;

// define structure 'rti_psi_struct' and type 'rti_psi_struct'
typedef struct rti_psi_struct {
  BIN4 nominal_sweep_angle;
  UINT4 starting_time_offset_from_sweep_time_in_ms;
  UINT4 ending_time_offset;
  BIN4 azimuth_angle_of_first_ray_in_file;
  BIN4 elevation_angle_of_first_ray_in_file;
  char ipad_end[PSI_SIZE-20];
} rti_psi_struct;

// define structure 'shear_psi_struct' and type 'shear_psi_struct'
typedef struct shear_psi_struct {
  BIN4 azimuth_smoothing_angle;
  BIN2 elevation_angle;
  SINT1 two_spare_bytes[2];
  UINT4 flag_word;
                   // Flag word (bits shared with sline_psi_struct)
                   // Bit0: do radial shear
                   // Bit1: do azimuthal shear
                   // Bit2: do mean wind correction to az shear using VVP
                   // Bit3: mean wind correction to azimuthal shear done
                   // Bit4: unfolding done in association with VVP product
                   // Bit5: do elevation shear
                   // Bit6: discard positive radial shear
                   // Bit7: do North-South shear
                   // Bit17: do East-West shear
#define SHEAR_FLG_RADIAL    (0x00000001)
#define SHEAR_FLG_AZIMUTHAL (0x00000002)
#define SHEAR_FLG_USE_VVP   (0x00000004)
#define SHEAR_FLG_VVP_USED  (0x00000008)
#define SHEAR_FLG_UNFOLDING (0x00000010)
#define SHEAR_FLG_ELEVATION (0x00000020)
#define SHEAR_FLG_DPRS      (0x00000040) /* Discard positive radial shear */
#define SHEAR_FLG_NORTH_SOUTH (0x00000080)
#define SHEAR_FLG_EAST_WEST   (0x00010000)
#define SHEAR_FLGS_ALL (SHEAR_FLG_RADIAL|SHEAR_FLG_AZIMUTHAL|SHEAR_FLG_USE_VVP| \
    SHEAR_FLG_VVP_USED|SHEAR_FLG_UNFOLDING|SHEAR_FLG_ELEVATION|SHEAR_FLG_DPRS| \
    SHEAR_FLG_NORTH_SOUTH|SHEAR_FLG_EAST_WEST)
  char name_of_VVP_product_to_use[12]; // space padded
  UINT4 maximum_age_of_VVP_to_use_in_secs;
  char  ipad_end[PSI_SIZE-28];
} shear_psi_struct;

// define structure 'sline_psi_struct' and type 'sline_psi_struct'
typedef struct sline_psi_struct {
  SINT4 area_in_square_meters;
  SINT4 shear_threshold_cm_per_sec_per_km;
  UINT4 bit_flags_to_choose_protected_areas;
  SINT4 maximum_forecast_time_in_seconds;
  UINT4 maximum_age_between_products_for_motion_calc;
  SINT4 maximum_velocity_allowed_in_motion;
  UINT4 flag_word;  /* SHEAR_FLG_* in here also */
#define SLINE_FLG_2ANGLES    (0x00000100)
#define SLINE_FLG_DIAGNOSTIC (0x00000200)
#define SLINE_FLG_MAXWRN     (0x00000400)
#define SLINE_FLG_2FCAST     (0x00000800)  /* Display with 2 forecast lines */
#define SLINE_FLGS_ALL (SLINE_FLG_2ANGLES|SLINE_FLG_DIAGNOSTIC| \
    SLINE_FLG_MAXWRN|SLINE_FLG_2FCAST|SHEAR_FLGS_ALL)
  BIN4 azimuthal_smoothing_angle; // 0 = none
  BIN4 elevation_angle;
  BIN4 elevation_angle_2;
  char name_of_VVP_task[12];
  UINT4 maximum_age_of_VVP_in_seconds;
  SINT4 curve_fit_standard_deviation_threshold_in_cm;
  UINT4 min_length_of_sline_in_tenths_of_km; // low byte, unsigned
  char  ipad_end[PSI_SIZE-64];
} sline_psi_struct;

// define structure 'sri_psi_struct' and type 'sri_psi_struct'
typedef struct sri_psi_struct {
  UINT4 flags;
#define SRI_FLG_PROFILE_CORRECTION (0x01) /* Apply the profile correction */
#define SRI_FLG_DIAGNOSTIC         (0x02) /* Make diagnostic product */
#define SRI_FLG_TERRAIN_MAP        (0x04) /* Use terrain map file */
#define SRI_FLG_MELT_SOURCE0       (0x08) /* Melting source: */
#define SRI_FLG_MELT_SOURCE1       (0x10) /* 0:Ingest, 1:Setup, 2:TypeIn */
#define SRI_FLG_CHECK_CONVECTION   (0x20) /* Check for convection */
#define SRI_FLG_NO_CLUTTER         (0x40) /* Clutter correction not applied */
#define SRI_FLG_NO_PROFILE         (0x80) /* Profile correction not applied */
#define SRI_FLGS_ALL   (SRI_FLG_PROFILE_CORRECTION|SRI_FLG_DIAGNOSTIC|SRI_FLG_TERRAIN_MAP|SRI_FLG_MELT_SOURCE0|SRI_FLG_MELT_SOURCE1|SRI_FLG_CHECK_CONVECTION|SRI_FLG_NO_CLUTTER|SRI_FLG_NO_PROFILE)
  SINT4 total_number_of_bins_inserted;
  SINT4 number_of_bins_with_data;
  SINT4 number_of_data_bins_profile_corrected;
  SINT2 surface_height_in_meters; // above reference
  SINT2 maximum_height_in_meters; // above reference
  SINT2 melting_height_in_meters; // above MSL
  SINT2 melting_level_thickness_in_m;
  SINT2 melting_level_intensity;
  SINT2 gradient_above_melting_per_100dB_per_km;
  SINT2 gradient_below_melting_per_100dB_per_km;
  SINT2 convective_check_height_in_meters; // above melting
  SINT2 convective_check_level; // DB_DBZ2 format
  char ipad_end[PSI_SIZE-34];
} sri_psi_struct;

// define structure 'tdwr_psi_struct' and type 'tdwr_psi_struct'
typedef struct tdwr_psi_struct {
  UINT4 flags; // Bit0=LLWAS; Bit1=WARN; Bit2=SLINE
#define TDWR_FLG_LLWAS    (0x0001)
#define TDWR_FLG_WARN     (0x0002)
#define TDWR_FLG_SLINE    (0x0004)
#define TDWR_FLG_2FCAST   (0x0008) /* Display with 2 forecast lines */
  UINT4 maximum_range_in_cm;
  char source_ID[4];
  char center_field_wind_direction[3];
  UINT1 spare_byte;
  char center_field_wind_speed[2];
  char center_field_gust_speed[2];
  UINT4 mask_of_protected_areas_checked;
  UINT4 number_of_centroids_in_file;
  UINT4 number_of_shear_lines_in_file;
  SINT4 forecast_time_in_seconds;
} tdwr_psi_struct;

// define structure 'top_psi_struct' and type 'top_psi_struct'
typedef struct top_psi_struct {
  UINT4 flags;
#define THICK_FLG_PSUEDO   (0x0001) /* Fake it if top or bottom missing */
  UINT2 z_threshold_in_sixteenths_of_dBz;
  char ipad_end[PSI_SIZE-6];
} top_psi_struct;

// define structure 'track_psi_struct' and type 'track_psi_struct'
typedef struct track_psi_struct {
  SINT4 centroid_area_threshold_in_square_meters;
  SINT4 threshold_level_for_centroid;
  UINT4 protected_area_mask;
  SINT4 maximum_forecast_time_in_seconds;
  UINT4 maximum_age_between_products_for_motion_calc;
  SINT4 maximum_motion_allowed_in_mm_per_second;
  UINT4 flag_word; // Bit9 = generate diagnostic output
#define TRACK_FLG_DIAGNOSTIC (0x00000200)
  SINT4 maximum_span_in_seconds_of_track_points_in_file;
  UINT4 input_product_type;
  char input_product_name[12];
  SINT4 point_connecting_error_allowance;
  char ipad_end[PSI_SIZE-52];
} track_psi_struct;

// define structure 'user_psi_struct' and type 'vad_psi_struct'
typedef struct user_psi_struct
{
  SINT4 imisc[PSI_SIZE/4];        /* Unspecified contents */
} user_psi_struct;

// define structure 'vad_psi_struct' and type 'vad_psi_struct'
typedef struct vad_psi_struct {
  SINT4 minimum_slant_range_in_cm;
  SINT4 maximum_slant_range_in_cm;
  UINT4 flags; // Bit0: Unfold based on VVP product;
#define VAD_FLG_UNFOLD  (0x0001)   /* Unfold based on VVP */
  UINT4 number_of_elevation_angles_in_file;
  char  ipad_end[PSI_SIZE-16];
} vad_psi_struct;

// define structure 'vil_psi_struct' and type 'vil_psi_struct'
typedef struct vil_psi_struct {
  SINT1 four_spare_bytes[4];
  SINT4 bottom_of_height_interval_in_cm;
  SINT4 top_of_height_interval_in_cm;
  char  ipad_end[PSI_SIZE-12];
} vil_psi_struct;

// define structure 'vvp_psi_struct' and type 'vvp_psi_struct'
typedef struct vvp_psi_struct {
#define VVP_UNFOLD     0x80000000 /* Use unfolding VVP algorithm */
#define VVP_HVEL       0x00000001 /* Horizontal Wind velocity and standard */
#define VVP_HVEL_STD   0x00000002 /*  deviation in cm/sec */
#define VVP_HDIR       0x00000004 /* Wind direction and standard deviation */
#define VVP_HDIR_STD   0x00000008 /*  in degrees*10 */
#define VVP_VVEL       0x00000010 /* Vertical speed and standard deviation */
#define VVP_VVEL_STD   0x00000020 /*  in cm/sec */
#define VVP_DIVERG     0x00000040 /* Horizontal divergence in 1E7/sec, and */
#define VVP_DIVERG_STD 0x00000080 /*  its standard deviation. */
#define VVP_RVEL_STD   0x00000100 /* Std deviation of radial velocity points*/
#define VVP_LINDBZ     0x00000200 /* Reflectivity in standard 2-byte format */
#define VVP_LOGDBZ_STD 0x00000400
#define VVP_DEFORM     0x00000800 /* Deformation in 1E7/sec, and its */
#define VVP_DEFORM_STD 0x00001000 /*  standard deviation. */
#define VVP_AXDIL      0x00002000 /* Axis of dilation (0-180 degrees) */
#define VVP_AXDIL_STD  0x00004000 /*  in degrees*10. */
#define VVP_RHOHV      0x00020000 /* RhoHV in standard 2-byte format */
#define VVP_RHOHV_STD  0x00040000
#define VVP_MAXIMUM_BITMASK (0x00020000) /* Highest bit in the regular list above */
#define VVP_ALL_PARAMS (VVP_HVEL | VVP_HVEL_STD | VVP_HDIR   | VVP_HDIR_STD | \
                        VVP_VVEL | VVP_VVEL_STD | VVP_DIVERG | VVP_DIVERG_STD | \
                        VVP_RVEL_STD | VVP_LINDBZ | VVP_LOGDBZ_STD | \
                        VVP_DEFORM | VVP_DEFORM_STD | \
                        VVP_AXDIL | VVP_AXDIL_STD | VVP_RHOHV | VVP_RHOHV_STD )
  SINT4 minimum_range_to_process_in_cm;
  SINT4 maximum_range_to_process_in_cm;
  SINT4 minimum_height_above_reference_to_process_in_cm;
  SINT4 maximum_height_above_reference_to_process_in_cm;
  SINT4 number_of_height_intervals_to_process;
  SINT4 target_number_of_bins_per_interval;
  UINT4 wind_parameters_to_compute; // bits defined in vvp_results struct
  UINT4 minimum_radial_velocity_in_cm_per_seconds;
  UINT4 maximum_horizontal_velocity_error_to_accept; // (DB_HVEL2)
  UINT4 minimum_sample_size;                         // (DB_HVEL2)
  UINT4 minimum_horizontal_velocity_to_accept;       // (DB_HVEL2)
  UINT4 maximum_horizontal_velocity_to_accept;       // (DB_HVEL2)
  UINT4 maximum_mean_reflectivity_to_accept;         // (DB_HVEL2)
  UINT4 maximum_vertical_velocity_to_accept;         // (DB_HVEL2)
} vvp_psi_struct;

// define structure 'warn_psi_struct' and type 'warn_psi_struct'
typedef struct warn_psi_struct {
#define WARN_MAX_INPUTS 3
  SINT4 centroid_area_threshold_in_square_meters;
  SINT4 threshold_levels_in_hundredths[WARN_MAX_INPUTS]; // hundredths of user units
  SINT2 data_valid_times_in_seconds[WARN_MAX_INPUTS];
  char two_spare_bytes[2];
  char symbol_to_display[12];
  char names_of_product_files[WARN_MAX_INPUTS][12];
  UINT1 product_types_used_as_input[WARN_MAX_INPUTS];
  UINT1 control_flags;
#define WARN_FLG_CMPLT0  0x01             /*   Perform a "less than" comparison each */
#define WARN_FLG_CMPLT1  0x02             /*   of these inputs if the bit is set. */
#define WARN_FLG_CMPLT2  0x04
#define WARN_FLG_DIAGNOSTIC (0x08)        /* Make diagnostic product */
#define WARN_FLGS_ALL (WARN_FLG_CMPLT0|WARN_FLG_CMPLT1|WARN_FLG_CMPLT2|WARN_FLG_DIAGNOSTIC)
  UINT4 protected_area_bit_flags;
} warn_psi_struct;

// define structure 'wind_psi_struct' and type 'wind_psi_struct'
typedef struct wind_psi_struct {
  SINT4 minimum_height_in_cm; // above reference
  SINT4 maximum_height_in_cm; // above reference
  SINT4 minimum_range_in_cm;
  SINT4 maximum_range_in_cm;
  SINT4 number_of_points_in_range;
  SINT4 number_of_points_in_azimuth;
  SINT4 sector_length_in_cm;
  BIN4 sector_width_angle;
  UINT4 flag_word; // Flag word: Bit0 = subtract mean wind
  UINT4 wind_parameters_mask_of_included_VVP;
#define WIND_MAX_RANGE_COUNT   (40)
#define WIND_MAX_AZIMUTH_COUNT (36)

  char  ipad_end[PSI_SIZE-40];
} wind_psi_struct;

// Next define the union 'product_specific_info' and the type 'psi_u'
typedef union product_specific_info {  // this union occupies a maximum of 80 bytes
  beam_psi_struct    beam;
  cappi_psi_struct   cappi;
  catch_psi_struct   catch;
  cross_psi_struct   cross;
  fcast_psi_struct   fcast;
  maximum_psi_struct max;
  ppi_psi_struct     ppi;
  rain_psi_struct    rain;
  raw_psi_struct     raw;
  rhi_psi_struct     rhi;
  rti_psi_struct     rti;
  shear_psi_struct   shear;
  sline_psi_struct   sline;
  sri_psi_struct     sri;
  tdwr_psi_struct    tdwr;
  top_psi_struct     top;
  track_psi_struct   track;
  user_psi_struct    user;
  vad_psi_struct     vad;
  vil_psi_struct     vil;
  vvp_psi_struct     vvp;
  warn_psi_struct    warn;
  wind_psi_struct    wind;
  char ipad[PSI_SIZE];                       // if USER, OTHER, TEXT,
} psi_u;

// Next define the structure 'color_scale_def' and the type 'csd_s'
typedef struct color_scale_def {
  UINT4 flags;
#define COLOR_SCALE_VARIABLE  (0x0100)
#define COLOR_SCALE_OVERRIDE  (0x0200)  /* Not in file */
#define COLOR_SCALE_TOP_SAT   (0x0400)  /* All values above top shown as top */
#define COLOR_SCALE_BOT_SAT   (0x0800)  /* All values below bottom shown as bottom */
#define COLOR_SCALE_HIGHLIGHT (0x1000)  /* Highlight one color */
#define COLOR_SCALE_SAVED_MASK (COLOR_SCALE_VARIABLE|COLOR_SCALE_OVERRIDE|\
  COLOR_SCALE_TOP_SAT|COLOR_SCALE_BOT_SAT|COLOR_SCALE_HIGHLIGHT)
  SINT4 starting_level;
  SINT4 level_step;
  SINT2 number_of_colors_in_scale;
  UINT2 set_number_and_color_scale_number; // set number in low byte
                                           // color scale number in high byte
  UINT2 starting_values_for_variable_levels[16];
} csd_s;

// Next define the structure 'structure_header' and the type 'shd_s'
typedef struct structure_header {
  SINT2 structure_identifier;
#define ST_TASK_CONF       (22)    /* Task_Configuration */
#define ST_INGEST_HDR      (23)    /* ingest_header */
#define ST_INGEST_DATA     (24)    /* INGEST_data_header */
#define ST_TAPE_INVEN      (25)    /* Tape inventory */
#define ST_PRODUCT_CONF    (26)    /* Product_configuration */
#define ST_PRODUCT_HDR     (27)    /* Product_hdr */
#define ST_TAPE_HEADER     (28)    /* Tape_header_record */
#define ST_VERSION_STEP    (20)    /* Step between version 1 and 2 */
  SINT2 format_version_number;
#define SVER_TASK_CONF_P     5     /* Task_Configuration */
#define SVER_INGEST_HDR_P    4     /* ingest_header */
#define SVER_INGEST_DATA_P   3     /* ingest_data_header */
#define SVER_PRODUCT_CONF_P  6     /* Product_configuration */
#define SVER_PRODUCT_HDR_P   8     /* Product_hdr */
  SINT4 bytes_in_entire_struct;
  SINT2 reserved;
  SINT2 flags; // bit0 = structure complete
#define HD_STRUCTURE_COMPLETE  (0x0001)      /*  Structure Complete */
} shd_s;

#define PCF_TASK_MINOR_SIZE (16)
#define PCF_QPE_ALGORITHM_SIZE (12)
// Next define the structure 'product_configuration' and the type 'pcf_s'
typedef struct  product_configuration {
        struct structure_header hdr;
        UINT2 product_type_code;
#define PROD_PPI        1       /*  PPI */
#define PROD_RHI        2       /*  RHI */
#define PROD_CAPPI      3       /*  CAPPI */
#define PROD_CROSS      4       /*  Cross section */
#define PROD_TOPS       5       /*  Echo tops */
#define PROD_TRACK      6       /*  Storm track */
#define PROD_RAIN1      7       /*  Precipitation 1 hour */
#define PROD_RAINN      8       /*  Precipitation n hour */
#define PROD_VVP        9       /*  Velocity Volume processing */
#define PROD_VIL        10      /*  Vertically Integrated Liquid */
#define PROD_SHEAR      11      /*  Wind shear */
#define PROD_WARN       12      /*  Warning (overlay) */
#define PROD_CATCH      13      /*  Rain catchments */
#define PROD_RTI        14      /*  Range-Time-Display */
#define PROD_RAW        15      /*  Raw data set (no display)*/
#define PROD_MAX        16      /*  Maximum with side panels */
#define PROD_USER       17      /*  Earth projection user product */
#define PROD_USERV      18      /*  Section projection user product */
#define PROD_OTHER      19      /*  Other user product (no display) */
#define PROD_STATUS     20      /*  Status product (no display) */
#define PROD_SLINE      21      /*  Shear Line Product */
#define PROD_WIND       22      /*  Horizontal wind field */
#define PROD_BEAM       23      /*  Beam pattern */
#define PROD_TEXT       24      /*  Text */
#define PROD_FCAST      25      /*  Forecast */
#define PROD_NDOP       26      /*  Multi-Doppler */
#define PROD_IMAGE      27      /*  Arbitrary graphics image */
#define PROD_COMP       28      /*  Composite */
#define PROD_TDWR       29      /*  TDWR Wind Alert */
#define PROD_GAGE       30      /*  Raingage product */
#define PROD_DWELL      31      /*  Dwell and bird detection product */
#define PROD_SRI        32      /*  Surface Rainfall Intensity */
#define PROD_BASE       33      /*  Echo bottoms */
#define PROD_HMAX       34      /*  Height of Max Reflectivity */
#define PROD_VAD        35      /*  Velocity azimuth display */
#define PROD_THICK      36      /*  Echo thickness */
#define PROD_SATELLITE  37      /*  Satellite data */
#define PROD_LAYER      38      /*  Layer average */
        UINT2 scheduling_code;
#define PSC_HOLD      0         /*  Do not run at all. */
#define PSC_NEXT      1         /*  Run once on next available data */
#define PSC_ALL       2         /*  Run as data becomes available */
#define PSC_AGAIN     3         /*  Run again on data last used */
#define PSC_ONCE      4         /*  Run once then remove from schedule */
        SINT4 seconds_to_skip_between_runs;
        struct ymds_time product_GenTime_UTC;
        struct ymds_time IngestSweep_input_time_TZ;
        struct ymds_time IngestFile_input_time_TZ;
        char spare_bytes[6];
        char product_configfile_name[12];
        char dataGen_task_name[12];
        UINT2 flag_word;
/* Flags in the file, and in the inventory */
#define PF_KEEP        (0x0020)  /* Do not remove this file    */
#define PF_CLUTTER     (0x0040)  /* This is a clutter map      */
#define PF_COMPOSITED  (0x0800)  /* This product has been composited*/
#define PF_DWELL       (0x1000)  /* This product is a dwell product*/
#define PF_ZR_SOURCE0  (0x2000)  /* Source of Z/R numbers: */
#define PF_ZR_SOURCE1  (0x4000)  /* 0:TypeIn, 1:Setup, 2:Disdro, 3:Unused */
/* Product type specific flags in the file, and in the inventory */
#define PF_WARN_TDWR   (0x0002)  /* Make TDWR style messages */
#define PF_WARN_SAY    (0x0080)  /* Speak warning messages */
/* Bits stored in the file */
#define PF_IN_FILE (PF_KEEP|PF_CLUTTER|PF_WARN_TDWR|PF_WARN_SAY|PF_COMPOSITED|PF_DWELL|PF_ZR_SOURCE0|PF_ZR_SOURCE1)
/* Bits controlled by PCF menu */
#define PF_IN_PCF  (PF_WARN_TDWR|PF_WARN_SAY|PF_ZR_SOURCE0|PF_ZR_SOURCE1)
/* Flags in the product schedule */
#define PF_RUNNING    (0x0004)  /* This pcf is being run               */
#define PF_HEADER     (0x0008)  /* Header, placeholder only, dont execute  */
/* Flags in the product inventory only */
#define PF_DELETE     (0x0010)  /* Delete this product        */
#define PF_REINGEST   (0x0100)  /* Tagged for reingesting     */
#define PF_TIMEOUT    (0x0200)  /* Status product has timed out*/
#define PF_PENDING    (0x0001)  /* Status product from before startup*/
/* Flags in server-to-client report */
#define PF_CHECKED    (0x0400)  /* Status product checking enabled for this site*/
        SINT4 X_scale_cm_per_pixel;
        SINT4 Y_scale_cm_per_pixel;
        SINT4 Z_scale_cm_per_pixel;
        SINT4 X_array_size;
        SINT4 Y_array_size;
        SINT4 Z_array_size;
#define MIN_PRODUCT_SIZE     (16)   /* Min/Max sizes of X & Y */
#define MAX_PRODUCT_SIZE   (3100)
#define MAX_PRODUCT_SURFACES (50)   /* CAPPI, MAX, etc., horizontal surface bound */
        SINT4 X_Radar_Location;
        SINT4 Y_Radar_Location;
        SINT4 Z_Radar_Location;
        SINT4 Max_Range_in_cm;
        UINT1 HydroClass;
        SINT1 spare_byte;
        UINT2 data_type_generated;
        char name_of_projection[12];
        UINT2 data_type_used_as_input;
        UINT1 projection_type_code;
        char  spare_byte_2;
        SINT2 radial_smoother_in_km_over_100;
        SINT2 number_of_runs_this_product;
        SINT4 Z_R_constant_thousandths;
        SINT4 Z_R_exponent_thousandths;
        SINT2 x_smoother_in_hundredths_of_km;
        SINT2 y_smoother_in_hundredths_of_km;
        psi_u product_specific_info;
        //  suffixes list is a Null terminated list of hybrid extensions
        char list_of_minor_task_suffixes[PCF_TASK_MINOR_SIZE];
        char QPE_algorithm_name[PCF_QPE_ALGORITHM_SIZE];
        csd_s colors;
} pcf_s;

// Next define the structure 'product_end' and the type 'ped_s'
typedef struct product_end {
        char site_name[16]; // where product was made, space padded
        char IRIS_version_product_maker[8]; // where product was made, null terminated
        char IRIS_version_ingest_data[8]; // where ingest data came from
        struct ymds_time time_of_oldest_input_ingest_file; // only RAIN1, RAINN, TZ flexible
        UINT1 spare_bytes_1[28];
        SINT2 minutes_LST_is_west_of_GMT;
        char hardware_name_of_ingest_data_source[16];  // space padded
        char site_name_of_ingest_data_source[16];  // space padded
        SINT2 minutes_recorded_standard_time_is_west_of_GMT;
        BIN4 latitude_of_center;
        BIN4 longitude_of_center;
        SINT2 signed_ground_height_relative_to_sea_level;
        SINT2 height_of_radar_above_the_ground_in_meters;
        SINT4 PRF_in_hertz;
        SINT4 pulse_width_in_hundredths_of_microseconds;
        UINT2 type_of_signal_processor_used;
        UINT2 trigger_rate_scheme;
        SINT2 number_of_samples_used;
        char name_of_clutter_filter_file[12];
        UINT2 number_of_linear_based_filter_for_the_first_bin;
        SINT4 wavelength_in_hundredths_of_centimeters;
        SINT4 truncation_height_in_cm_above_radar;
        SINT4 range_of_the_first_bin_in_cm;
        SINT4 range_of_the_last_bin_in_cm;
        SINT4 number_of_output_bins;
        UINT2 flag_word;
/*
 * If the Disdrometer was selected as the Z/R source, but it was not
 * available, then we switch to setup source, and set this bit.
 */
#define PEF_ZR_FALLBACK (0x0001)
        SINT2 number_of_ingest_or_product_files_used; // only on RAIN1 and RAINN
        UINT2 type_of_polarization_used;
        SINT2 IO_cal_value_horizontal_pol_in_hundredths_of_dBm;
        SINT2 noise_at_calibration_horizontal_pol_in_hundredths_of_dBm;
        SINT2 radar_constant_horizontal_pol_in_hundredths_of_dB;
        UINT2 receiver_bandwidth_in_kHz;
        SINT2 current_noise_level_horizontal_pol_in_hundredths_of_dBm;
        SINT2 current_noise_level_vertical_pol_in_hundredths_of_dBm;
        SINT2 LDR_offset_in_hundredths_dB;
        SINT2 ZDR_offset_in_hundredths_dB;
        UINT2 TFC_cal_flags; // see struct task_calib_info
        UINT2 TFC_cal_flags2; // see struct task_calib_info
        char spare_bytes_2[18];
        BIN4 projection_angle_standard_parallel_1;
        BIN4 projection_angle_standard_parallel_2;
        UINT4 Equatorial_radius_of_earth_in_cm; // zero = 6371km sphere
        UINT4 one_over_flattening_in_millionths; // zero = sphere
        UINT4 fault_status_of_task; // see ingest_configuration
        UINT4 mask_of_input_sites_used_in_a_composite;
        UINT2 number_of_log_based_filter_for_the_first_bin;
        UINT2 nonzero_if_cluttermap_applied_to_the_ingest_data;
        BIN4 latitude_of_projection_reference;
        BIN4 longitude_of_projection_reference;
        SINT2 product_sequence_number;
        char spare_bytes_3[32];
        SINT2 melting_level_in_meters; // 0=unknown, most-sig-byte complimented
        SINT2 height_of_radar_in_meters; // above reference height
        SINT2 number_of_elements_in_product_results_array;
        UINT1 mean_wind_speed;
        UINT1 mean_wind_direction; // unknown if both speed and direction are zero
        char spare_bytes_4[2];
        char time_zone_name_of_recorded_data[8];
        UINT4 offset_to_extended_time_header;
        char spare_bytes_5[4];
} ped_s;

/* Next define the structure 'ingest_configuration' and the type 'icf_s' */
typedef struct ingest_configuration {
  char name_of_file_on_disk[80];
  SINT2 number_of_associated_disk_files_extant; // this number increases with # of sweeps
  SINT2 number_of_sweeps_completed;
  SINT4 total_size_of_all_files; // files in INGEST directory
  struct ymds_time time_that_volume_scan_was_started; // see TZ at byte 166 and 224
  char twelve_spare_bytes[12];
  SINT2 number_of_bytes_in_ray_headers;
  SINT2 number_of_bytes_in_extended_ray_headers; // includes normal ray header
  SINT2 number_of_bytes_in_task_configuration_table;
  SINT2 playback_version_number;
  char four_spare_bytes[4];
  char IRIS_version_number[8]; // 8 bytes, null terminated
  char ingest_hardware_name_of_site[16]; // 16 bytes
  SINT2 minutes_west_of_GMT_of_LST; // Time zone of local standard time
  char radar_site_name_from_setup_utility[16]; // 16 bytes
  SINT2 minutes_west_of_GMT_recorded_time; // Time zone pertaining to recorded data
  BIN4 latitude_of_radar;
  BIN4 longitude_of_radar;
  SINT2 height_of_ground_site_in_meters_above_sea_level;
  SINT2 radar_height_in_meters_above_ground;
  UINT2 resolution_as_rays_per_360_degree_sweep;
  UINT2 index_of_first_ray; // implies angle of first
  UINT2 number_of_rays_in_sweep;
  SINT2 bytes_in_each_gparam;
  SINT4 altitude_of_radar_cm_above_sea_level;
  SINT4 velocity_of_radar_in_cm_per_sec_east_north_up[3];
  SINT4 antenna_offset_from_INU_in_cm_starboard_bow_up[3];
  UINT4 fault_status; // at the time the task was started
#define ICFB_BITE_FAULT      (0x00000001) /* Normal BITE fault */
#define ICFB_BITE_CRITICAL   (0x00000002) /* Critical BITE fault */
#define ICFB_RCP_FAULT       (0x00000004) /* Normal RCP fault */
#define ICFB_RCP_CRITICAL    (0x00000008) /* Critical RCP fault */
#define ICFB_SYS_CRITICAL    (0x00000010) /* Critical system fault */
#define ICFB_PRODUCT_FAULT   (0x00000020) /* Product generator faulted */
#define ICFB_OUTPUT_FAULT    (0x00000040) /* Output master faulted */
#define ICFB_SYS_FAULT       (0x00000080) /* Normal system fault */
  SINT2 height_of_melting_level_above_sea_level_in_meters; // MSB is complemented, zero=UNKNOWN
  char two_spare_bytes[2];
  char local_timezone_string[8]; // null terminated
  UINT4 flags; // Bit0=First ray not centered on zero degrees
#define ICF_FLG_ANGLE_EDGE   (0x00000001) /* First ray not centered on zero */
  char config_name_in_the_dpolapp_conf_file[16]; // null terminated configuration name
  char two_hundred_twenty_eight_spare_bytes[228];
} icf_s;

// define the structure 'enum_convert' and the type 'ecv_s'
typedef struct enum_convert {
    UINT1 id_of_active_echo_classifier; // implicit type enum classifiers
    UINT1 bit_offset_of_the_enum_segment;
    UINT1 length_of_the_enum_segment_in_bits;
    char one_spare_byte;
} ecv_s;

// define the structure 'task_end_info' and the type 'tei_s'
typedef struct task_end_info {
  /* The task number consists of a "major" and "minor" number.  The
   * major number is what we usually refer to the task by.  The minor
   * number is used for "hybrid" tasks i.e., when more than one task
   * configuration is used to define an overall task.  If the minor
   * number is zero, then there are no additional tasks. Otherwise the
   * minor numbers represent an ordering of a set of task
   * configurations, all of which have the same major number. The
   * minor numbers always run from 1...N, where N is the total number
   * of tasks involved in this hybrid.
   */
  SINT2 task_major_number;
  SINT2 task_minor_number;
  /* There are two character strings associated with a task.  One is
   * the task name, i.e., file name of the .TCO file.  The other is an
   * optional brief description of what the task does.  Both strings
   * are end-padded with spaces.
   */
  char name_of_task_configuration_file[12];
  char eighty_byte_task_description[80];
  SINT4 number_of_tasks_in_this_hybrid_set;
  // STATE is the current state of the task.
  // It can be modified by the menus
  // or INGEST processes.
  UINT2 task_state;
#define TASK_VOID         0     // No task defined here (empty slot)
#define TASK_MODIFY       1     // Being modified by someone
#define TASK_INACTIVE     2     // Exists, but not used in any way
#define TASK_SCHED        3     // Waiting to run
#define TASK_RUNNING      4     // Running
  char  two_spare_bytes[2];
  struct ymds_time task_time;    // Task time, set when running, TZ flexible
  char two_hundred_four_bytes_spare[204];
} tei_s;

// define the structure 'task_misc_info' and the type 'tmi_s'
typedef struct task_misc_info {
  SINT4 radar_wavelength_in_cm_x100;
  char serial_number_of_transmitter[16]; // User's id of xmitter, receiver, etc, space term
  SINT4 transmit_power_in_watts;
  UINT2 iflags; // Bit0: digital signal simulator in use Bit1: Polarization in use; Bit4: Keep bit
#define TASK_MSC_DSSIM       0x0001     /* Digital Signal Simulator in use */
#define TASK_MSC_PARTIAL     0x0002     /* Volume scan was halted prematurely */
#define TASK_MSC_KEEP        0x0010     /* Keep this file (WATCHDOG info) */
#define TASK_MSC_CLUTMAP     0x0020     /* File tagged as a clutter map (INGEST info) */
  UINT2 type_of_polarization; // see dsp_lib.h
  SINT4 truncation_height_in_cm_above_radar;
  char  eighteen_bytes_reserved[18];          // Reserved for polarization description
  char twelve_bytes_spare[12];
  /* Users comments are associated with tasks.  The comment buffer
   * itself is part of the Task Configuration Structure, and the
   * number of bytes inserted in that buffer so far is given below.
   */
  SINT2 number_of_bytes_of_comments_entered;
  BIN4  horizontal_beam_width;  // Horizontal full beamwidth
  BIN4  vertical_beam_width;    // Vertical full beamwidth
  UINT4 iUser[10];              // customer defined storage
  char two_hundred_eight_bytes_spare[208];
} tmi_s;

// define the structure 'task_rhi_scan_info' and the type 'trsi_s'
typedef struct task_rhi_scan_info {
  // --- RHI Azimuth List ---
  //   Starting and ending elevation angles, followed by
  //   AZ list of binary angles
  BIN2 Lower_elevation_angle_limit;  // Angle limits for sector scans only
  BIN2 Upper_elevation_angle_limit;
  UINT2 list_of_azimuth_angles_to_scan[MAX_SWEEPS];
  char one_hundred_fifteen_bytes_spare[115];
#define TASK_SCAN_RHI_NEAREST  (0)
#define TASK_SCAN_RHI_LOWER    (1)
#define TASK_SCAN_RHI_UPPER    (2)
  UINT1 iStartEnd; // Which end of the sector to start at
                   // 0=Nearest, 1=Lower, 2=Upper
                   // sector sweeps alternate in direction
} trsi_s;

// define the structure 'task_ppi_scan_info' and type 'tpsi_s'
typedef struct task_ppi_scan_info {
  // --- PPI Elevation List ---
  // Actual # of items in IELLST followed by
  // EL list of binary angles
  BIN2  left_azimuthal_angle_limit;   // Angle limits for sector scans only
  BIN2  right_azimuthal_angle_limit;
  UINT2 list_of_elevation_angles_to_scan[MAX_SWEEPS];
  char one_hundred_fifteen_bytes_spare[115];
#define TASK_SCAN_PPI_NEAREST  (0)
#define TASK_SCAN_PPI_LEFT     (1)
#define TASK_SCAN_PPI_RIGHT    (2)
  UINT1 iStartEnd;  // Which end of the sector to start at
                    // 0=Nearest, 1=Lower, 2=Upper
                    // sector sweeps alternate in direction
} tpsi_s;

// define the structure 'task_file_scan_info' and type 'tfsi_s'
typedef struct task_file_scan_info {
  // --- File Scan Info ---
  // First azimuth and elevation from the file, followed by
  // the file name.
  UINT2 first_azimuth_angle;   // binary angle
  UINT2 first_elevation_angle; // binary angle
  char file_name_for_antenna_control[12]; // Space terminated
  char one_hundred_eighty_four_bytes_spare[184];
} tfsi_s;

// define the structure 'task_manual_scan_info' and type 'tmsi_s'
typedef struct task_manual_scan_info {
#define TCF_SCAN_MAN_CONT_MASK (0x0001)
#define TCF_SCAN_CONT_MASK     (0x0001)
#define TCF_SCAN_SET_AZ        (0x0002)
#define TCF_SCAN_SET_EL        (0x0004)
  UINT2 flags;             // flags Bit0: continuous recording*/
  char  two_spare_bytes[2];
  BIN4  first_azimuth_angle;
  BIN4  first_elevation_angle;
  char ipad_end[188];
} tmsi_s;

// define the structure 'task_exec_scan_info' and type 'tesi_s'
typedef struct task_exec_scan_info {
  char sCommand[160];   // character array holding commands to execute
                        // Null terminated
  char fourty_bytes_spare[40];
} tesi_s;

// define the union 'task_scan_info_u' and the type 'tsi_u'
typedef union task_scan_info_u
{
  struct task_rhi_scan_info rhi;
  struct task_ppi_scan_info ppi;
  struct task_file_scan_info fil;
  struct task_manual_scan_info man;
  struct task_exec_scan_info exec;
} tsi_u;

// define the structure 'task_scan_info' and the type 'tscani_s'
typedef struct task_scan_info
{
  UINT2 antenna_scan_mode;   // Antenna Scan Mode (TASK_SCAN_xxx)
                             // 1:PPI sector
                             // 2:RHI sector
                             // 3:Manual
                             // 4:PPI full
                             // 5:file
                             // 6:exec
                             // 7:RHI full

  /* The next item is the desired angular resolution expressed as an integer
   * number of thousandths of degrees.  This format was chosen (rather
   * than binary angle) so that user-units of resolution could be
   * expressed exactly.  In manual scans, this is the number of rays
   * to record.
   */
  SINT2 angular_resolution_x1000;

  /* In the PPI scan modes, SCAN_SPEED indicates the azimuth rotation
   * rate in binary angles per second.  If this value is initially
   * zero, then INGEST will calculate a scan rate based on the other
   * task parameters and overwrite the zero with the computed
   * value.
   */
  BIN2 scan_speed;

  SINT2 number_of_sweeps_to_perform;  // # Sweeps to perform

  // if RHI scan use                          <task_rhi_scan_info>
  // if PPI sector or PPI continuous scan use <task_ppi_scan_info>
  // if file scan use                         <task_file_scan_info>
  // if PPI manual or RHI manual use          <task_manual_scan_info>
  union task_scan_info_u u;
  char one_hundred_twelve_byte_spare[112];
} tscani_s;

// define the structure 'task_range_info' and the type 'tri_s'
typedef struct  task_range_info  {
  SINT4 range_of_first_bin_in_cm; // Range of first (input) bin in cm
  SINT4 range_of_last_bin_in_cm;  // Range of last (input) bin in cm
  SINT2 number_of_input_range_bins;
  SINT2 number_of_output_range_bins;
  SINT4 step_between_input_bins_in_cm;
  SINT4 step_between_output_bins_in_cm;

  /* If variation_of_range_bin_spacing_flag is zero, then the input
   * bins are equally spaced, in which case the input and output bin
   * spacings, and any bin averaging and smoothing are defined.  If
   * variation_of_range_bin_spacing_flag is non-zero, then it
   * indicates that some type of variable input spacing has been
   * selected.  In that case, there are (up to) 48 bytes reserved
   * to describe the variable format specifically.
   */
  UINT2 variation_of_range_bin_spacing_flag;  /* Non-Zero ==> variable resolution */
  SINT2 averaging_of_range_bin_spacing;  /* 0:No Avg,  1:Avg Pairs, ... */
  SINT2 smoothing_of_range_bin_spacing;  /* 0:No smoothing,  ... */
  char one_hundred_thirty_four_bytes_spare[134];
} tri_s;

// define the structure 'task_calib_info' and the type 'tci_s'
typedef struct task_calib_info {
  // Various calibration slopes and thresholds
  SINT2 reflectivity_slope; // LOG slope in dB*4096/ A/D count
  SINT2 reflectivity_noise_threshold; // LOG noise threshold in dB*16
  SINT2 clutter_correction_threshold; // Clutter Correction threshold in dB*16
  SINT2 SQI_threshold;               // SQI threshold (0->1) * 256
  SINT2 signal_power_thresholdr;    // Signal power threshold in dBm*16
  SINT2 pmi_threshold;               // PMI threshold * 256
  char  six_bytes_spare[6];
  SINT2 calibration_reflectivity; // Calibration reflectivity in dBZ*16 at 1km
#define TCF_CAL_ZCALIB_SCALE  (16.0)
  // Threshold control flags (TCF) for various parameters
#define TCFTERM_NONE    0       /* The TCFTERM_xx constants are used to */
#define TCFTERM_LOG     1       /*   specify a collection of threshhold */
#define TCFTERM_CSR     2       /*   qualifiers for a given parameter.  */
#define TCFTERM_SQI     3       /*   Up to four my be combined as the  */
#define TCFTERM_WSP     4       /*   nybbles of a 16-bit TCFMASK word.  */
#define TCFTERM_PMI     5
#define TCFTERM_COUNT     6     /* Count of total number of terms */
#define TCFTERM_STRINGS { "---", "LOG", "CSR", "SQI", "SIG", "PMI" }

#define TCFMASK_COMPAT 0x0000   /* For backward compatibility */
#define TCFMASK_LEGACY 0x4321   /* Legacy WSP/SQI/CSR/LOG combination */

#define TCF_Q0    0xAAAA        /* The four TCFMASK terms are combined  */
#define TCF_Q1    0xCCCC        /*   in an arbitrary Boolean expression */
#define TCF_Q2    0xF0F0        /*   to qualify each parameter.         */
#define TCF_Q3    0xFF00

#define TCF_LOG   TCF_Q0        /* Legacy threshold control flags, only   */
#define TCF_CSR   TCF_Q1        /*   for TCFMASK_LEGACY or TCFMASK_COMPAT */
#define TCF_SQI   TCF_Q2
#define TCF_WSP   TCF_Q3

#define TCF_ALL_PASS              0xFFFF
#define TCF_ALL_FAIL              0x0000
#define TCF_LOG_AND_CSR           0x8888
#define TCF_LOG_AND_SQI           0xA0A0
#define TCF_LOG_AND_CSR_AND_SQI   0x8080
#define TCF_LOG_AND_SIG_AND_SQI   0xA000
#define TCF_SQI_OR_LOG            0xFAFA
#define TCF_SQI_AND_CSR           0xC0C0
#define TCF_SQI_AND_SIG           0xF000
#define TCF_SQI_AND_SIG_AND_CSR   0xC000
#define TCF_SQI_OR_SIG            0xFFF0
#define TCF_SQI_OR_SIG_AND_CSR    0xCCC0
#define DSPTCF_IOR_TERMS 4      /* Max # of OR'ed terms */

  UINT2 flags_for_uncorrected_reflectivity; // UnCorrected Z flags
  UINT2 flags_for_corrected_reflectivity;   // Corrected Z flags
  UINT2 flags_for_velocity;                // Velocity flags
  UINT2 flags_for_width;                // Width flags
  UINT2 flags_for_zdr;               // ZDR flags
  char  six_spare_bytes_2[6];
  // Miscellaneous processing flags
  UINT2 flags;
#define TCF_CAL_ZSPECKLE      0x0001 // Log channel speckle remover ON
#define TCF_CAL_VSPECKLE      0x0004 // Linear channel speckle remover ON
#define TCF_CAL_RANGENORM     0x0010 // Data is range normalized
  /* #define TCF_CAL_BEG_PULSE     0x0020 // DSP issues pulse at beginning of rays */
  /* #define TCF_CAL_END_PULSE     0x0040 // DSP issues pulse at end of rays */
#define TCF_CAL_VAR_PULSES    0x0080 // DSP varies # pulses in Dual PRF mode
#define TCF_CAL_3LAG_WIDTHS   0x0100 // Use 3-lag Doppler processing (else 2)
#define TCF_CAL_SHIP_COR      0x0200 // Velocities corrected for ship motion
#define TCF_CAL_VC_UNFOLD     0x0400 // Vc has unfolding via VVP
#define TCF_CAL_VC_FALLSPD      0x0800 // Vc has fall speed correction
#define TCF_CAL_ZC_BEAMBLOCK    0x1000 // Zc has Partial beam blockage correction
#define TCF_CAL_ZC_ATTENUATION  0x2000 // Zc has Z Intervening attenuation correction
#define TCF_CAL_ZC_TARGET_DET   0x4000 // Zc has Target Detection correction
#define TCF_CAL_VC_STORM_RELATIVE 0x8000 // Vc has storm relative velocity correction
#define TCF_CAL_USED_MASK (TCF_CAL_ZSPECKLE      | TCF_CAL_VSPECKLE       | \
                           TCF_CAL_RANGENORM     | TCF_CAL_VAR_PULSES     | \
                           TCF_CAL_3LAG_WIDTHS   | TCF_CAL_SHIP_COR       | \
                           TCF_CAL_VC_UNFOLD     | TCF_CAL_VC_FALLSPD     | \
                           TCF_CAL_ZC_BEAMBLOCK  | TCF_CAL_ZC_ATTENUATION | \
                           TCF_CAL_ZC_TARGET_DET | TCF_CAL_VC_STORM_RELATIVE)
  char two_spare_bytes[2] ;
  SINT2 ldr_bias_in_dBx100;              // LDR bias in dB*100 (XDR)
  SINT2 zdr_bias_in_dBx16;              // ZDR bias in dB*16  (GDR)
  SINT2 point_clutter_threshold_in_dBx100; // Threshold in dB*100
  UINT2 point_clutter_bin_skip; // Side skip in low 4 bits, 0=feature off
  SINT2 I0_cal_value_Horiz_in_hundredths_of_dB; // I0 number from calibration, 1/100 of dB
  SINT2 I0_cal_value_Vert_in_hundredths_of_dB;
  SINT2 noise_at_calibration_Horiz_in_hundredths_of_dBm; // Noise at calib time, 1/100 of dBm
  SINT2 noise_at_calibration_Vert_in_hundredths_of_dBm;
  SINT2 radar_constant_Horiz_in_hundredths_of_dB;
  SINT2 radar_constant_Vert_in_hundredths_of_dB;
  UINT2 receiver_bandwidth_in_kHz;
  UINT2 flags2;
#define TCF_CAL2_ZC_DP_ATTEN     0x0001 /* Zc and ZDRc has DP attenuation correction */
#define TCF_CAL2_Z_DP_ATTEN      0x0002 /* Z and ZDR has DP attenuation correction */
#define TCF_CAL2_2DSPECKLE       0x0004 /* 2D (3x3) speckle speckle/unfold   */
#define TCF_CAL2_USED_MASK (TCF_CAL2_ZC_DP_ATTEN | TCF_CAL2_Z_DP_ATTEN | TCF_CAL2_2DSPECKLE)
  // Threshold mechanisms/choices; TCFTTERM_xxx / TiTCFMask of dsp.h
  UINT2 uncorrected_reflectivity_tcfMask; // UnCorrected Z threshold mechanisms
  UINT2 corrected_reflectivity_tcfMask;   // Corrected Z threshold mechanisms
  UINT2 velocity_tcfMask;          // Velocity threshold mechanisms
  UINT2 width_tcfMask;             // Width threshold mechanisms
  UINT2 zdr_tcfMask;               // ZDR threshold mechanisms
//  UINT2 polarimetric_phas_tcfMask; // Polarimetric phases threshold mechanisms
  char two_hundred_fourty_six_spare_bytes[246];
} tci_s;

// define the structure 'dsp_data_mask' and type 'dsp_data_mask'
//(holding information about data types)
typedef struct dsp_data_mask {
  UINT4 dWord0;
  UINT4 iXhdrType;
  UINT4 dWord1;
  UINT4 dWord2;
  UINT4 dWord3;
  UINT4 dWord4;
} dsp_data_mask;

// define the structure 'task_dsp_mode_batch' and type 'task_dsp_mode_batch'
typedef struct task_dsp_mode_batch {
  UINT2 low_PRF_in_hz;         // Low PRF in Hz
  UINT2 low_PRF_fraction_part; // scaled by 2**-16
  SINT2 low_PRF_sample_size;
  SINT2 low_PRF_range_averaging_in_bins; // Averaging applied to low PRF
  SINT2 threshold_reflectivity_unfolding_in_hundredths_dB;
  SINT2 threshold_velocity_unfolding_in_hundredths_dB;
  SINT2 threshold_width_unfolding_in_hundredths_dBd;      /* Width dB threshold (1/100 dB ) */
  char eighteen_spare_bytes[18];
} task_dsp_mode_batch;

// define the structure 'task_dsp_mode_other' and type 'task_dsp_mode_other'
typedef struct task_dsp_mode_other
{
  SINT2 imisc[16];
} task_dsp_mode_other;

// define the structure 'task_dsp_mode_u' and the type 'tdm_U'
typedef union task_dsp_mode_u {
  task_dsp_mode_batch batch;
  task_dsp_mode_other other;
} tdm_u;

// define the structure 'task_dsp_info' and the type 'tdi_s'
typedef struct task_dsp_info {
  UINT2 dsp_major_mode ;           // DSP mode (from dsp_lib.h)
/*
 * Major mode of the signal processor (RVP6 and greater).  Values
 * for bit field within the opprm Flag/AGC word.
 */
#define PMODE_PPP     (0x0)     /* Pulse Pair Doppler */
#define PMODE_FFT     (0x1)     /* FFT Doppler */
#define PMODE_RPH     (0x2)     /* Random Phase Doppler */
#define PMODE_KNMI    (0x3)     /* KNMI custom LOG clutter filter */
#define PMODE_DPT1    (0x4)     /* Dual-PRT Type-1 (for low duty cycle) */
#define PMODE_DPT2    (0x5)     /* Dual-PRT Type-2 (for velocity unfolding) */
#define PMODE_BATCH   (0x6)     /* Legacy WSR88D batched trigger/unfolding */
#define PMODE_ADPDF   (0x7)     /* Adaptive Dual-Pulse Dual-Frequency */
#define PMODE_USER1   (0xC)     /* User-defined modes */
#define PMODE_USER2   (0xD)
#define PMODE_USER3   (0xE)
#define PMODE_USER4   (0xF)
#define PMODE_MAX_COUNT (16)     /* Maximum possible number of processing modes */


  UINT2 dsp_type ;                 // DSP type (from dsp_lib.h)
#define DSP_TYPE_RVP6      (5)   /* RVP6 Processor */
#define DSP_TYPE_RVP6_2    (6)   /* RVP6 Dual board Processor */
#define DSP_TYPE_RVP7      (7)   /* RVP7 Processor */
#define DSP_TYPE_RVP8      (8)   /* RVP8 Processor */
#define DSP_TYPE_RVP9      (9)   /* RVP900 Processor */
  /* The type(s) of data being recorded by the DSP are summarized in
   * DataMask.  IDATA is an array of 160 bit positions,
   * indicating which types of data are to be ingested.
   * For DB_XHDR, the iXhdrType gives information about which type
   * of extended header data is recorded.
   * For definitions see the dsp_lib.h file.
  */
  /* UINT4 idata; */
  struct dsp_data_mask DataMask;
  /* We can optionally block recording of some data types when
   * making the RAW product.  This records the original data types.
   * Unused if all zero.  Added on 2-Nov-2004 in release 8.07.
   */
  struct dsp_data_mask OriginalDataMask;
  // in next union
  // if Batch Major mode then
  //           <task_dsp_mode_batch>
  // else
  //           <task_dsp_mode_other>
  union task_dsp_mode_u u;
  char  fifty_two_spare_bytes[52];
  // Trigger and related Info
  SINT4 prf_in_hertz;
  SINT4 pulse_width_in_hundredths_of_microseconds;
  UINT2 multi_PRF_mode_flag;  // 0=1:1, 1=2:3, 2=3:4, 3=4:5 see dsp_lib.h
#define PRF_FIXED   (0)         /* Fixed trigger rate */
#define PRF_2_3     (1)         /* 2:3 PRF Ratio */
#define PRF_3_4     (2)         /* 3:4 PRF Ratio */
#define PRF_4_5     (3)         /* 4:5 PRF Ratio */
#define PRF_SEQ0    (4)         /* Custom sequence 0 */
#define PRF_SEQ1    (5)         /* Custom sequence 1 */
#define PRF_SEQ2    (6)         /* Custom sequence 2 */
#define PRF_SEQ3    (7)         /* Custom sequence 3 */
  SINT2 dual_PRF_delay;       // Stabilization # pulses for multi-PRF
  UINT2 agc_feedback_code;    // Selected coefficient for AGC feedback
  // Miscellaneous
  SINT2 sample_size;          // number of pulses used
  UINT2 gain_control_flag;
#define GAIN_FIXED     0        /* Fixed receiver gain */
#define GAIN_STC       1        /* STC Gain */
#define GAIN_AGC       2        /* AGC Gain */
  // File defining clutter filter.  If the file name is blank, then
  // the first filter code is used at all ranges.
  char name_of_file_used_for_clutter_filter[12]; // Space terminated
  UINT1 clutter_filter_index; // Spectral clutter filter number used
  UINT1 log_filter_first_bin;      // Z based filter (unused, for legacy RVP6 data only)
  SINT2 fixed_gain; // 1000 * fixed gain level (0-1) ?
  UINT2 gas_attenuation ;          // 100000 * db/km, up to 10000, thereafter 10 times slope
  UINT2 flag_nonzero_if_clutter_map_used;  // Clutter map applied to DSP data
  UINT2 xmt_phase_sequence; // Phase sequence to use, if major_mode=PMODE_RPH, 0:fixed, 1:random, 3:SZ8/64
  UINT4 CfgHdr_Mask; // Mask sent to CFGHDR command (to configure the ray header) in signal processor
  UINT2 flags_time_series_playback;  // Time series playback related flags, see OPTS_*
  char  two_spare_bytes[2];
  // If the DataMask.iXhdrType number above is 2, then use this custom header name.
  char  name_of_custom_ray_header[16];
  // List of what is in the HydroClass word
  struct enum_convert enums[6];
  char ninety_six_spare_bytes[96];
} tdi_s;


// define the structure 'task_sched_info' and the type 'tschedi_s'
typedef struct task_sched_info {
  /* There are six times stored for each task.  All times are in
   * seconds offset from the start of a 24-hour day.  Invalid times
   * are indicated by a value of -1.
   *
   * START and STOP are absolute times that define when the task can
   * be running at all.  SKIP is the desired time between individual
   * runs. Each time a task is run the beginning time is stored in
   * LAST_RUN.  When a run finishes, the time used is written to
   * TIME_USED.  LAST_RUN_DAY is a relative day number that is used to
   * resolve very old last run times.
   */
  SINT4 start_time_seconds_within_day; // Start time for task
  SINT4 stop_time_seconds_within_day;  // Stop time for task
  SINT4 desired_skip_time_in_seconds;  // Desired skip between runs
  SINT4 last_run_seconds_within_day;   // Time that task was last run, 0 for passive ingest
  SINT4 time_used_on_last_run_seconds; // Time used on last run, in file time to writeout
  SINT4 relative_day_of_last_run;       // Day that task was last run, days since 1970, 0 for passive ingest
  /* The flag word modifies the scheduling details.
   *  * ASAP indicates that the scheduling times should be ignored and that the
   *    task should be run once as soon as possible, after which the ASAP bit
   *    will be cleared.
   *  * MAND indicates that the task can preempt a non-MAND task that is already
   *    running, and also defines a separate level of priority for normal time-
   *    based scheduling.
   *  * LSKIP is a late skip flag which indicates that the task must run within
   *    a certain tolerance of its expected starting times, else it will be
   *    skipped.
   *  * STOP indicates to INGEST that the task should be descheduled after
   *    running it, i.e., its state should go to INACTIVE, rather than SCHEDULED.
   */
  UINT2 iflag;
#define TASK_SCH_ASAP      (0x0001) /* Start as_soon_as_possible */
#define TASK_SCH_MAND      (0x0002) /* Task has mandatory status */
#define TASK_SCH_LSKIP     (0x0004) /* Late Skip */
#define TASK_SCH_MESTU     (0x0008) /* TIME_USED is measured, else estimated. */
#define TASK_SCH_DESCHED   (0x0010) /* De-schedule task after running it. */
#define TASK_SCH_INTR      (0x0020) /* Interrupt this task right now */
#define TASK_SCH_FLIP      (0x0040) /* Offer to flip control to other system */
  char ninety_four_bytes_spare[94];
} tschedi_s;

// define the structure 'task_configuration' and the type 'tcf_s'
typedef struct task_configuration {
  struct structure_header    hdr;   //  12 bytes
  struct task_sched_info     sch;   // 120 bytes
  struct task_dsp_info       dsp;   // 320 bytes
  struct task_calib_info     cal;   // 320 bytes
  struct task_range_info     rng;   // 160 bytes
  struct task_scan_info      scan;  // 320 bytes
  struct task_misc_info      misc;  // 320 bytes
  struct task_end_info       end;   // 320 bytes
  //
  // The comment buffer is the last part of the Task Configuration Stucture.
  //
  char comnts[ TASK_COMNT_SIZE ];   // 720 bytes
} tcf_s;

// define the structure 'gparm' and the type 'gpa_s'
typedef struct gparm {
  UINT2 irev_ser ;              /* 01: Rev/Serial # */
  UINT2 ibin_out_num ;          /* 02: # range bins output by processor */
  UINT2 iprt_mes ;              /* 03: Current trigger period */
  UINT2 itaga ;                 /* 04: Tag bits 00 - 15 */
  UINT2 itagb ;                 /* 05: Tag bits 16 - 31 */
  UINT2 log_nse ;               /* 06: Log Channel Noise */
  SINT2 i_nse_ ;                /* 07: I Channel Noise */
  SINT2 q_nse_ ;                /* 08: Q Channel Noise */
  UINT2 istat_l ;               /* 09: Latched status */
/* Bits in GPARM latched status word */
#define GLS_NTRIGNSE     0x0001 /* No trigger during noise measurement */
#define GLS_FASTNSE      0x0002 /* Trig too fast during noise measurement */
#define GLS_NTGPROC      0x0004 /* No trigger during PROC command */
#define GLS_PRFVARIED    0x0008 /* PRF varied from beginning to end of ray */
#define GLS_POLERROR     0x0010 /* Error in polarization control/status */
#define GLS_FIFOERROR    0x0020 /* Buffer overflow during PROC command */
#define GLS_OLOST        0x0040 /* Some user output words were lost */
#define GLS_NOISERROR    0x0080 /* Error detected during noise measurement */
#define GLS_RMASK        0x0200 /* Range mask error */
#define GLS_SIMULATE     0x0400 /* Simulated data sequencing error */
#define GLS_PHSEQMES     0x0800 /* Measured phase sequence is incorrect */
#define GLS_NOLICENSE    0x2000 /* Attempt to use an unlicensed feature */
#define GLS_ZLINSE       0x4000 /* Error in linear Z noise calculation */
#define GLS_PMODERR      0x8000 /* Configuration error during PROC command */
  UINT2 istat_i ;               /* 10: Immediate status */
/* Bits in GPARM immediate status word (#1) */
#define GIS_NOTRIG       0x0001 /* No trigger */
#define GIS_SYLDERR      0x0002 /* Error loading sync angle table */
#define GIS_PWINFO       0x0004 /* PWINFO command is enabled */
#define GIS_SYBCD        0x0008 /* Sync: angles are BCD */
#define GIS_SYEL         0x0010 /* Sync: angles are for elevation axis */
#define GIS_SYENAB       0x0020 /* Sync: enabled */
#define GIS_SYSHORT      0x0040 /* Sync: allow rays from short pulse groups */
#define GIS_SYDYNAM      0x0080 /* Sync: dynamic ray width (else table bounds) */
#define GIS_SYOPBITS     0x00F8 /* The above four bits */
#define GIS_HASIAGC      0x0100 /* An AUX board with IAGC is connected */
#define GIS_16BITTS      0x0200 /* 16-bit time series is supported */
#define GIS_UMODE0       0x0400 /* 2-bit unfolding mode */
#define GIS_UMODE1       0x0800
#define GIS_NAUX0        0x1000 /* Count of # of AUX boards attached */
#define GIS_NAUX1        0x2000
#define GIS_SPEC         0x4000 /* Power Spectrum output is supported */
  UINT2 idiag_a ;               /* 11: Diagnostic Results A */
  UINT2 idiag_b ;               /* 12: Diagnostic Results B */
  UINT2 isamp ;                 /* 13: # pulses per ray */
  UINT2 itrg_cnt_a ;            /* 14: Low 16 bits of trigger count */
  UINT2 itrg_cnt_b ;            /* 15: High 8 bits of trigger count */
  UINT2 iaqbins ;               /* 16: # properly acquired bins */
  UINT2 iprbins ;               /* 17: # properly processed bins */
  UINT2 istat_i2 ;              /* 18: Immediate Status (second word) */
/* Bits in GPARM immediate status word (#2) */
#define GI2S_FFT         0x0001 /* FFT processing mode is supported */
#define GI2S_RPH         0x0002 /* Random phase mode is supported */
#define GI2S_KNM         0x0004 /* KNMI LOG filtering is supported */
#define GI2S_DPT1        0x0008 /* Dual-PRT Type-1 is supported */
#define GI2S_UPLERR      0x0010 /* Error in uplink to receiver */
#define GI2S_DNLERR      0x0020 /* Downlink error / no signal detected */
                                /* 0x0040 was: separate noise levels per PW */
#define GI2S_PLLERR      0x0080 /* Error in user clock PLL in receiver */
#define GI2S_AFCBITS     0x0700 /* Current AFC status code */
#define GI2S_AFCLOWBIT   8
#define AFC_DISABLED  1
#define AFC_MANUAL    2
#define AFC_NOBURST   3
#define AFC_WAIT      4
#define AFC_LOCKED    5
#define AFC_TRACK     6
#define GI2S_IFDSWS      0x0800 /* Receiver's switches not set to "run" */
#define GI2S_TBLANK      0x1000 /* Trigger blanking is enabled */
#define GI2S_NOBURST     0x2000 /* Missing signal at IFD burst input */
#define GI2S_DSPCOUNT    0x4000 /* DSP count may be less than what's available */
#define GI2S_SLIPBURST   0x8000 /* Burst pulse energy is not well centered */
  UINT2 inse_rng ;              /* 19: Noise range in Km */
  UINT2 inse_prt ;              /* 20: Noise period */
  UINT2 ipwmin_0 ;              /* 21: Pulse width 0 min trigger period */
  UINT2 ipwmin_1 ;              /* 22: Pulse width 1 min trigger period */
  UINT2 ipwmin_2 ;              /* 23: Pulse width 2 min trigger period */
  UINT2 ipwmin_3 ;              /* 24: Pulse width 3 min trigger period */
  UINT2 ipw_bits ;              /* 25: Pulse width bit patterns */
  UINT2 ipw_now ;               /* 26: Current pulsewidth choice */
#define GPARM_PW_PULSE1_MASK   (0x000f)
#define GPARM_PW_PULSE1_SHIFT  (0)
#define GPARM_MAJOR_MODE_MASK  (0x00f0)
#define GPARM_MAJOR_MODE_SHIFT (4)
#define GPARM_WINDOW_MASK      (0x0700)
#define GPARM_WINDOW_SHIFT     (8)
#define GPARM_HAS_PULSE2_FLAG  (0x0800)
#define GPARM_PW_PULSE2_MASK   (0xf000)
#define GPARM_PW_PULSE2_SHIFT  (12)
  UINT2 iprt_gen ;              /* 27: Current trigger generator period */
  UINT2 iprt_des ;              /* 28: Desired trigger generator period */
  UINT2 iprt_start ;            /* 29: Measured PRT at start of last ray */
  UINT2 iprt_end ;              /* 30: Measured PRT at end of last ray */
  UINT2 iflags ;                /* 31: Processing/threshold flags */
  SINT2 iz_slope ;              /* 32: LOG conversion Slope */
  SINT2 izns_thr ;              /* 33: LOG noise threshold */
  SINT2 iccr_thr ;              /* 34: Clutter correction threshold */
  UINT2 isqi_thr ;              /* 35: SQI threshold */
  SINT2 isig_thr ;              /* 36: Signal Power threshold */
  SINT2 iz_calib ;              /* 37: Calibration Reflectivity */
  UINT2 iqi_now ;               /* 38: (Q,I) current A/D sample */
  UINT2 iz_now ;                /* 39: LOG current A/D sample */
  UINT2 ibin_avg ;              /* 40: Range averaging choice (0:None, 1:Pairs, etc) */
  UINT2 idiag_c ;               /* 41: Reserved for future diagnostic bits */
  UINT2 idiag_d ;               /* 42: Reserved for future diagnostic bits */
  UINT2 iproc_hdr0 ;            /* 43: Bits 0-15 defining header of PROC rays */
  UINT2 isq_lo ;                /* 44: Noise sample average value of I squared, */
  SINT2 isq_hi ;                /* 45:   expressed as a 32-bit quantity. */
  UINT2 qsq_lo ;                /* 46: Same for Q... */
  SINT2 qsq_hi ;                /* 47:  */
  SINT2 zlin_noise ;            /* 48: Linearized LOG power noise sample mean */
  SINT2 zlin_rms  ;             /* 49: Linearized LOG power noise root-mean-square */
  SINT2 inse_hv_ratio ;         /* 50: Horiz/Vert noise ratio in dB*100 */
  SINT2 iafclevel ;             /* 51: Signed 16-bit AFC level */
  UINT2 intflt ;                /* 52: Interference filter (bits  3:0) */
                                /*           Minor Version (bits  7:4) */
                                /*      IFD Sat.Power Code (bits 11:8) */
  SINT2 intflt_p1 ;             /* 53:Interference filter parameter #1 */
  SINT2 intflt_p2 ;             /* 54:Interference filter parameter #1 */
  UINT2 istat_i3 ;              /* 55: Immediate Status (third word) */
/* Bits in GPARM immediate status word (#3) */
#define GI3S_BPTIMEADJ   0x0001 /* Burst Pulse time adjustments are okay */
#define GI3S_BPFREQADJ   0x0002 /* Burst Pulse frequency adjustments are okay */
#define GI3S_BPHUNTENA   0x0004 /* Burst Pulse hunting is enabled */
#define GI3S_BPHUNTNOW   0x0008 /* Hunting is running right now */
#define GI3S_BPHFAIL     0x0010 /* Last hunt failed to find the burst */
#define GI3S_DPT2        0x0020 /* Dual PRT (Type 2) is supported */
#define GI3S_PHSEQGEN    0x0040 /* Could not generate the requested phase seq */
#define GI3S_TXCLK       0x0080 /* Problem with digital transmitter clock */
#define GI3S_USER1       0x0100 /* Extended user PMODEs are supported */
#define GI3S_USER2       0x0200
#define GI3S_USER3       0x0400
#define GI3S_USER4       0x0800
/*                       0x1000  Reserved for GI3S_USER5 */
/*                       0x2000  Reserved for GI3S_USER6 */
/*                       0x4000  Reserved for GI3S_USER7 */
/*                       0x8000  Reserved for GI3S_USER8 */

  SINT2 itrigslew ;             /* 56: Burst tracking slew (usec * 100) */
  UINT2 iPolFlags ;             /* 57: Polarization flags */
#define POLFLAG_TX_H     0x0001 /*       Use H transmissions for (T,Z,V,W) */
#define POLFLAG_TX_V     0x0002 /*       Use V transmissions for (T,Z,V,W) */
#define POLFLAG_RX_CO    0x0004 /*       Use Co-Pol reception for (T,Z,V,W) */
#define POLFLAG_RX_CX    0x0008 /*       Use Cross-Pol reception for (T,Z,V,W) */
#define POLFLAG_CNOISE   0x0010 /*       Correct polar power params for noise */
#define POLFLAG_FILTER   0x0020 /*       Use filtered data for all polar params */
#define POLFLAG_PHISIGN  0x0040 /*       Sign convention for PHIdp */
#define POLFLAG_DPATTEN  0x0080 /*       Z and ZDR are corrected for attenuation using PhiDP */
#define POLFLAG_CNOISECOV 0x0100 /*       Correct polar phase params for noise */
  UINT2 iMaskSpacingCM ;        /* 58: Range mask spacing (cm) for current pulsewidth */
  UINT2 istat_i4 ;              /* 59: Immediate Status (fourth word) */
/* Bits in GPARM immediate status word (#4) */
#define GI4S_ANYSPECINT  0x0001 /* Power spectra for params matches sample size */
#define GI4S_ANYSPECOUT  0x0002 /* PROC output spectra match sample size */
#define GI4S_ALTPATTERN  0x0004 /* Bits indicating that the trigger */
#define GI4S_ALTPERIOD   0x0008 /*   pattern and/or PRT were altered. */
#define GI4S_IQPACKHISNR 0x0010 /* Using High-SNR packed (I,Q) format */
#define GI4S_ALTTRUNC    0x0020 /* Out-of-Memory, triggers truncated */
#define GI4S_EXTERNALTS  0x0040 /* TimeSeries data source is external to the DSP */
#define GI4S_BATCH       0x0080 /* Batch processing mode is supported */
#define GI4S_CANTEXTRIG  0x0100 /* Major mode refuses to use external trigger */
#define GI4S_NSEFMTPACK  0x0200 /* Use packed format for i_nse and q_nse */
#define GI4S_RXPROTFLT   0x0400 /* Receiver protection fault */
#define GI4S_IFDCHANERR  0x0800 /* IFD dual-channel inconsistency */
#define GI4S_GPS1PPSERR  0x1000 /* GPS 1-pulse-per-second clock error */
  UINT2 unused_word_60 ;        /* 60:  */
  UINT2 unused_word_61 ;        /* 61:  */
  UINT2 unused_word_62 ;        /* 62:  */
  UINT2 unused_word_63 ;        /* 63:  */
  UINT2 unused_word_64 ;        /* 64:  */
} gpa_s;

// Next define the structure 'ingest_data_header' and the type 'idh_s'
typedef struct ingest_data_header {
  struct structure_header hdr; // the size stored here is total size of the sweep or
                               // ingest file. That is, 76 + 4*Irtotl +Iwrit*raysize
  struct ymds_time sweep_start_time; // Date and time that sweep was started
  SINT2 sweep_number;                // Sweep number origin is 1
  /* The following three variables are copies of variables of the same name from
   * the ingest_configuration structure.  There are included here for convenience when
   * processing data files.
  */
  SINT2 resolution_as_rays_per_360_degree_sweep;
  SINT2 index_of_first_ray; // implies angle of first
  SINT2 number_of_rays_in_sweep;
  /* IWRITN gives the number of rays actually written in this file, i.e., the
   * number of non-zero ray pointers.  It is possible that not all of the pointers
   * are actually filled in, e.g., if the antenna scan speed was too fast, some
   * angles might have been missed.
  */
  SINT2 rays_written;           // # rays written [0 -> number_of_rays_in_sweep]
  BIN2 fixed_angle_of_sweep;    // Fixed angle for this sweep
  SINT2 number_of_bits_per_bin; // bits/bin for these data
  UINT2 data_type;              // Data code, 1=total power in dBz, etc (See Task_DSP_Info.IDATA)
  char thirty_six_bytes_spare[36];
} idh_s;

// Next define the structure 'raw_prod_bhdr' and the type 'rpb_s'
typedef struct raw_prod_bhdr {
  SINT2 record_number;              //  Record # within the file, origin is 0
  SINT2 sweep_number;               //  Sweep # origin is 1
  SINT2 offset_of_first_ray_in_record; //  Byte offset (from BHdr) of first ray, -1 if none
  SINT2 ray_number_within_sweep;       //  Ray number of above first ray
  UINT2 flags;                     // only one so far; Bit 0: block has invalid data
#define APB_NOGOOD    (0x0001)      /*    Block has invalid data */
  UINT1 two_spare_bytes[2];
} rpb_s;


// Next define the structure 'product_hdr' and the type 'phd_s'
typedef struct product_hdr {
  struct structure_header hdr;          // Generic Header       12 bytes
  struct product_configuration pcf;     // Product Config Info 320 bytes
  struct product_end end;                       // Product End Info    308 bytes
} phd_s;

// Next define the structure 'ingest_header' and the type 'ihd_s'
typedef struct ingest_header {
  struct structure_header hdr;          // Generic Header       12 bytes
  struct ingest_configuration icf;      // Ingest Config Info  480 bytes
  struct task_configuration tcf;        // Task Config        2612 bytes
  char spare[732];
  struct gparm GParm;                   //                      128 bytes
                                       // Read from the RVP just after configuration
  char reserved[920];
} ihd_s;

#endif
