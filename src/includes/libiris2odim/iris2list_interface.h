/* --------------------------------------------------------------------
Copyright (C) 2015 The Crown (i.e. Her Majesty the Queen in Right of Canada)

This file is an add-on to RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/
/**
 * Description
 * @file
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-11-25
 */
#ifndef IRIS2LIST_INTERFACE_H
#define IRIS2LIST_INTERFACE_H
#include "irisdlist.h"


sweep_element_s *handle_ingest_data_headers(
                    IrisDList_t **sweeplist,
                    sweep_element_s **sweep_list_element,
                    IRISbuf *IRISbuf_p,
                    _Bool target_is_big_endian);

rhd_s *extract_ray_header(UINT1 *ptr_s0);

rayplus_s *extract_rayplus(IRISbuf **IRISbuf_pp,
                             UINT2 offset,
                             IrisDList_t **sweeplist_pp,
                   sweep_element_s **sweep_list_element_pp,
                             SINT2 current_sweep,
                              FILE *fp,
                             _Bool target_is_big_endian );

phd_s *extract_product_hdr(IRISbuf *IRISbuf_p,
                           _Bool target_is_big_endian);

shd_s *extract_structure_header(UINT1 *s1, _Bool target_is_big_endian);

pcf_s *extract_product_configuration(UINT1 *s1,
                                      _Bool target_is_big_endian);

ped_s *extract_product_end(UINT1 *s1, _Bool target_is_big_endian);

ihd_s *extract_ingest_header(IRISbuf *IRISbuf_p,
                             _Bool target_is_big_endian);

icf_s *extract_ingest_configuration(UINT1 *s1,
                                     _Bool target_is_big_endian);

tcf_s *extract_task_configuration(UINT1 *s0,
                                   _Bool target_is_big_endian);

gpa_s *extract_gparm(UINT1 *s0,
                      _Bool target_is_big_endian);

tscani_s *extract_task_scan_info(UINT1 *s1,
                                  _Bool target_is_big_endian);

tschedi_s *extract_task_sched_info(UINT1 *s1,
                                   _Bool target_is_big_endian);

tdi_s *extract_task_dsp_info(UINT1 *s1,
                              _Bool target_is_big_endian);

tci_s *extract_task_calib_info(UINT1 *s1,
                                _Bool target_is_big_endian);

tmi_s *extract_task_misc_info(UINT1 *s1,
                               _Bool target_is_big_endian);

tri_s *extract_task_range_info(UINT1 *s1,
                                _Bool target_is_big_endian);

tei_s *extract_task_end_info(UINT1 *s1,
                              _Bool target_is_big_endian);

tpsi_s *extract_task_ppi_scan_info(UINT1 *s1,
                                    _Bool target_is_big_endian);

trsi_s *extract_task_rhi_scan_info(UINT1 *s1,
                                    _Bool target_is_big_endian);

tmsi_s *extract_task_manual_scan_info(UINT1 *s1,
                                       _Bool target_is_big_endian);

tfsi_s *extract_task_file_scan_info(UINT1 *s1,
                                     _Bool target_is_big_endian);

tesi_s *extract_task_exec_scan_info(UINT1 *s1);

dsp_data_mask *extract_dsp_data_mask(UINT1 *s1,
                                      _Bool target_is_big_endian);

rpb_s *extract_raw_prod_bhdr(IRISbuf *IRISbuf_p,
                             _Bool target_is_big_endian);

csd_s *extract_color_scale_def(UINT1 *s1, _Bool target_is_big_endian);
  

IRISbuf *getabuf(FILE *fp, UINT2 bytes2Copy);

ymd_s *extract_ymds_time(UINT1 *s1,
                          _Bool target_is_big_endian);

SINT2 Swap2BytesSigned( UINT2 shortIn);

SINT4 Swap4BytesSigned( UINT4 intIn);

ecv_s *extract_enum_convert(UINT1 *s1);


ppi_psi_struct *extract_psi_ppi(UINT1 *s1,
                                _Bool target_is_big_endian);
rhi_psi_struct *extract_psi_rhi(UINT1 *s1,
                                _Bool target_is_big_endian);
cappi_psi_struct *extract_psi_cappi(UINT1 *s1,
                                    _Bool target_is_big_endian);
cross_psi_struct *extract_psi_cross(UINT1 *s1,
                                    _Bool target_is_big_endian);
top_psi_struct *extract_psi_tops(UINT1 *,
                                 _Bool target_is_big_endian);
track_psi_struct *extract_psi_track(UINT1 *s1,
                                    _Bool target_is_big_endian);
rain_psi_struct *extract_psi_rain(UINT1 *s1,
                                  _Bool target_is_big_endian);
vvp_psi_struct *extract_psi_vvp(UINT1 *s1,
                                _Bool target_is_big_endian);
vil_psi_struct *extract_psi_vil(UINT1 *s1,
                                _Bool target_is_big_endian);
shear_psi_struct *extract_psi_shear(UINT1 *s1,
                                    _Bool target_is_big_endian);
warn_psi_struct *extract_psi_warn(UINT1 *s1,
                                  _Bool target_is_big_endian);
catch_psi_struct *extract_psi_catch(UINT1 *s1,
                                    _Bool target_is_big_endian);
rti_psi_struct *extract_psi_rti(UINT1 *s1,
                                _Bool target_is_big_endian);
raw_psi_struct *extract_psi_raw(UINT1 *s1,
                                _Bool target_is_big_endian);
maximum_psi_struct *extract_psi_max(UINT1 *s1,
                                    _Bool target_is_big_endian);
sline_psi_struct *extract_psi_sline(UINT1 *s1,
                                    _Bool target_is_big_endian);
wind_psi_struct *extract_psi_wind(UINT1 *s1,
                                  _Bool target_is_big_endian);
beam_psi_struct *extract_psi_beam(UINT1 *s1,
                                  _Bool target_is_big_endian);
fcast_psi_struct *extract_psi_fcast(UINT1 *s1,
                                    _Bool target_is_big_endian);
tdwr_psi_struct *extract_psi_tdwr(UINT1 *s1,
                                  _Bool target_is_big_endian);
user_psi_struct *extract_psi_user(UINT1 *s1,
                                  _Bool target_is_big_endian);
sri_psi_struct *extract_psi_sri(UINT1 *s1,
                                _Bool target_is_big_endian);

idh_s *extract_ingest_data_header(IRISbuf *IRISbuf_p,
                                  UINT2 offset,
                                  _Bool target_is_big_endian);

void deep_copy_product_header(phd_s *from, file_element_s **file_element_pp);
void deep_copy_ingest_header(ihd_s *from, file_element_s **file_element_pp);

#endif
