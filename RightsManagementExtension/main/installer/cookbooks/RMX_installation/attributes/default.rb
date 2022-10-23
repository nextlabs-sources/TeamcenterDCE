# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Attribute     :: Default Attributes
# Description   :: Create attributes container to some common values used by recipe in cookbook
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

default['temp_dir'] = ''
default['install_dir'] = ''
default['TC_ROOT'] = ''
default['TC_DATA'] = ''
default['DISP_ROOT'] = ''
# default['bb_dir'] = ''
# default['scf_enable'] = false

default['tc_version'] = '10'
default['tc_full_version'] = ''
default['MAX_VERSION_NUM'] = 99_999

default['tc_profilevar'] = ''

default['service_tcserver'] = ''
default['aws_runtime'] = false

default['jpc_eva_port'] = 1099

default['infodba_psd'] = ''
default['tc_user_psd'] = ''
default['skydrm_app_key'] = ''
default['cc_apisecret'] = ''
default['OLD_RMX_VERSION'] = ''
default['template_upgrade'] = ''

default['feature_list'] = []
