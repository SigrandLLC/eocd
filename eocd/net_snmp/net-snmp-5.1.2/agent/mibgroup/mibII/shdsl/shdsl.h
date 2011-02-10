/*
 *  hdsl2,shdsl MIB (rfc- interfaces.h
 */

#ifndef _MIBGROUP_HDSL2SHDSL_H
#define _MIBGROUP_HDSL2SHDSL_H

//---- header functions ----//
static int header_dslIfIndex(struct variable*, oid *,size_t *,
							 int,size_t*,WriteMethod**,int *);

/*---- callbacks ----*/
extern FindVarMethod var_SpanConfEntry;
extern FindVarMethod var_SpanStatusEntry;
extern FindVarMethod var_InventoryEntry;
/*
  extern FindVarMethod var_EndpointConfEntry;
*/

extern FindVarMethod var_EndpointCurrEntry;

extern FindVarMethod var_15MinIntervalEntry;
extern FindVarMethod var_1DayIntervalEntry;
/*
  extern FindVarMethod var_EndpointMaintEntry;
  extern FindVarMethod var_UnitMaintEntry;
  // profiles
  */
extern FindVarMethod var_SpanConfProfEntry;

/*
  extern FindVarMethod var_EndpointAlermEntry;
  extern FindVarMethod var_NotificationEntry;
*/

/*---- Magic values ----*/
/*
 * var_SpanConfEntry Magic numbers
 * Represents hdsl2ShdslSpanConfTable:
 * - table indexed by {ifIndex}
 * - each entry represents the complete span in single SHDSL line
 * - information is persistant
 */
#define CONF_NREPS	0
#define CONF_PRFL	1
#define CONF_ALARMPRFL	2

/*
 * var_SpanStatusEntry Magic numbers
 * Represents hdsl2ShdslSpanStatusTable:
 * - table indexed by {ifIndex},
 * - each entry represents the complete span in single SHDSL line
 * - information is NOT persistent
 */
#define STATUS_NAVAILREPS	0
#define STATUS_MAXATTLRATE	1
#define STATUS_ACTLRATE		2
#define STATUS_TRNSMSNMODCUR	3
#define STATUS_MAXATTPRATE	4
#define STATUS_ACTPRATE		5

/*
 * var_InventoryEntry Magic numbers
 * Represents hdsl2ShdslInventoryTable:
 * - table indexed by {ifIndex,hdsl2ShdslInvIndex}
 * - info retreive via EOC from units in SHDSL line
 * - each entry represents inventory information for a single unit in
 SHDSL line
 * - information is NOT persistent
 */
#define INV_INDEX	0
#define INV_VID		1
#define INV_VMODELNUM	2
#define INV_VSERNUM	3
#define INV_VEOCSV	4
#define INV_STANDARDV	5
#define INV_VLISTNUM	6
#define INV_VISSUENUM	7
#define INV_VSOFTWV	8
#define INV_EQCODE	9
#define INV_VOTHER	10
#define INV_TRNSMODECPB	11


/*
 * var_EndpointConfEntry Magic numbers
 * Represents hdsl2ShdslEndpointConfTable:
 * - table indexed by
 {ifIndex, hdsl2ShdslInvIndex,hdsl2ShdslEndpointSide,
 hdsl2ShdslEndpointWirePair}
 * - Configuration of alarm profile is setted by user
 * - represents a single segment endpoint in an HDSL2/SHDSL line
 * - information is persistent
 */
#define ENDP_SIDE	1
#define ENDP_PAIR	2
#define ENDP_CONF_PROF	3


/*
 * var_EndpointCurrEntry Magic numbers
 * Represents hdsl2ShdslEndpointCurrTable:
 * - table indexed by
 {ifIndex, hdsl2ShdslInvIndex,hdsl2ShdslEndpointSide,
 hdsl2ShdslEndpointWirePair}
 * - contains current status and performance information
 for segment endpoints in HDSL2/SHDSL lines
 * - information is persistent
 */

#define ENDP_STAT_CUR_ATN	3
#define ENDP_STAT_CUR_SNRMGN	4
#define ENDP_STAT_CUR_STATUS	5
#define ENDP_STAT_CUR_ES	6
#define ENDP_STAT_CUR_SES	7
#define ENDP_STAT_CUR_CRC	8
#define ENDP_STAT_CUR_LOSWS	9
#define ENDP_STAT_CUR_UAS	10
#define ENDP_STAT_CUR_15MEL	11
#define ENDP_STAT_CUR_15M_ES	12
#define ENDP_STAT_CUR_15M_SES	13
#define ENDP_STAT_CUR_15M_CRC	14
#define ENDP_STAT_CUR_15M_LOSWS	15
#define ENDP_STAT_CUR_15M_UAS	16
#define ENDP_STAT_CUR_1DEL	17
#define ENDP_STAT_CUR_1D_ES	18
#define ENDP_STAT_CUR_1D_SES	19
#define ENDP_STAT_CUR_1D_CRC	20
#define ENDP_STAT_CUR_1D_LOSWS	21
#define ENDP_STAT_CUR_1D_UAS	22


/*
 * var_Endpoint15minEntry Magic numbers
 * Represents hdsl2Shdsl15MinIntervalTable:
 * - table indexed by
 {ifIndex, hdsl2ShdslInvIndex,hdsl2ShdslEndpointSide,
 hdsl2ShdslEndpointWirePair,hdsl2Shdsl15MinIntervalNumber}
 * - contains history of performance information for segment endpoints in
 HDSL2/SHDSL lines
 * - information is NOT persistent
 */


#define ENDP_15M_INT 1
#define ENDP_15M_ES 2
#define ENDP_15M_SES	3
#define ENDP_15M_CRC	4
#define ENDP_15M_LOSWS	5
#define ENDP_15M_UAS	6


/*
 * var_Endpoint1dayEntry Magic numbers
 * Represents hdsl2Shdsl1DayIntervalTable:
 * - table indexed by
 {ifIndex, hdsl2ShdslInvIndex,hdsl2ShdslEndpointSide,
 hdsl2ShdslEndpointWirePair,hdsl2Shdsl1DayIntervalNumber}
 * - contains current status and performance information
 for segment endpoints in HDSL2/SHDSL lines
 * - information is persistent
 */

#define ENDP_1D_INT	1
#define ENDP_1D_MONSECS 2
#define ENDP_1D_ES	3
#define ENDP_1D_SES	4
#define ENDP_1D_CRC	5
#define ENDP_1D_LOSWS	6
#define ENDP_1D_UAS	7



/*
 * var_EndpointMaintEntry Magic numbers
 * Represents hdsl2ShdslEndpointMaintTable:
 * - table indexed by
 *	{ifIndex, hdsl2ShdslInvIndex,hdsl2ShdslEndpointSide}
 * - supports maintenance operations (e.g., loopbacks)
 *   to be performed on HDSL2/SHDSL segment endpoints
 * - information is persistent
 */

#define ENDP_MAINT_LOOPBACK 	1
#define ENDP_MAINT_TIPRINGREV	2
#define ENDP_MAINT_PWRBACKOFF	3
#define ENDP_MAINT_SOFTRESTART	4

/*
 * var_UnitMaintEntry Magic numbers
 * Represents hdsl2ShdslEndpointMaintTable:
 * - table indexed by
 *	{ ifIndex,hdsl2ShdslInvIndex }
 * - supports maintenance operations (e.g., loopbacks)
 *   to be performed on HDSL2/SHDSL segment unit
 * - information is persistent
 */

#define UNIT_MAINT_LPB_TO 	1
#define UNIT_MAINT_PWR_SRC	2


/*
 * var_SpanConfProfEntry Magic numbers
 * Represents hdsl2ShdslSpanConfProfileTable
 * - table indexed by
 *	{ hdsl2ShdslSpanConfProfileName }
 * - SHDSL Line configuration profile table to be
 *   performed on HDSL2/SHDSL segment unit
 * - information is persistent
 */

#define CONF_WIRE_IFACE 	1
#define CONF_MIN_LRATE	 	2
#define CONF_MAX_LRATE	 	3
#define CONF_PSD	 	4
#define CONF_TRNSM_MODE 	5
#define CONF_REM_ENABLE 	6
#define CONF_PWR_FEED	 	7
#define CONF_CURR_DOWN	 	8
#define CONF_WORST_DOWN 	9
#define CONF_CURR_UP	 	10
#define CONF_WORST_UP	 	11
#define CONF_USED_MARG	 	12
#define CONF_REF_CLK	 	13
#define CONF_LPROBE	 	14
#define CONF_ROW_ST	 	15

#endif                          /* _MIBGROUP_HDSL2SHDSL_H */
