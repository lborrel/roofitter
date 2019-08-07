#! /bin/bash
#
# This script was written by the satellite release manager
#  - Normally you should not edit it.
#  - The first line in this block is used as key by the gridExport
#    script to identify satellite releases - do not edit it.
#

# Connect to the base release.
if [[ "${MU2E_GRID_BASE_RELEASE_OVERRIDE}" == '' ]]; then
  source /cvmfs/mu2e.opensciencegrid.org/Offline/v7_5_3/SLF7/prof/Offline/setup.sh
else
  source ${MU2E_GRID_BASE_RELEASE_OVERRIDE}/setup.sh
fi
retstat=$?

if [[ "$retstat" != "0" ]]; then
  return $retstat;
fi

# This variable contains the physical path to the directory
# that contains this file  (regardless of cwd when this script is sourced ).
SCRIPTDIR=`dirname $(readlink -f $BASH_SOURCE)`

# Add the satellite release to path variables.
export MU2E_SATELLITE_RELEASE=$SCRIPTDIR
export MU2E_SEARCH_PATH=`dropit -p $MU2E_SEARCH_PATH -sf $MU2E_SATELLITE_RELEASE`
export FHICL_FILE_PATH=`dropit -p $FHICL_FILE_PATH -sf $MU2E_SATELLITE_RELEASE`

# paths needed to run from this satellite release
export CET_PLUGIN_PATH=`dropit -p $CET_PLUGIN_PATH -sf $MU2E_SATELLITE_RELEASE/lib`
export LD_LIBRARY_PATH=`dropit -p $LD_LIBRARY_PATH -sf $MU2E_SATELLITE_RELEASE/lib`
export PYTHONPATH=`dropit -p $PYTHONPATH -sf $MU2E_SATELLITE_RELEASE/scripts/build/python`
export PATH=`dropit -p $PATH -sf $MU2E_SATELLITE_RELEASE/bin`
export ROOT_INCLUDE_PATH=`dropit -p $ROOT_INCLUDE_PATH -sf $MU2E_SATELLITE_RELEASE`

