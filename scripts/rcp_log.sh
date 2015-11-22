#! /usr/local/bin/bash
##################################
#rcp_log.sh
#rcp access logs from coll servers
#-1. copy in rotation 
#-2. create a tmp file until downloaded
#-3. rename the file
#kmcgowan 20060821
################################
DELAY_H=1
DATE=`date -v -${DELAY_H}H +%Y%m%d`        
BLOCK=`date -v -${DELAY_H}H "+%Y%m%d-%H%M"`
#cmd line args
#DATE=$1  #ie: 2060101
#BLOCK=$2 #ie: 20060101-0000

BASEDIR=/mnt/disk/sort/logs
LOG=${BASEDIR}/rcp_log/rcp_${DATE}.log
LOCKFILE="/var/tmp/`basename $0`_${BLOCK}.pid"
LOGP="/usr/LOGS/${DATE}/"

function log()
{
   NOW=`date "+%Y%m%d %H:%M:%S"`
   echo "$NOW: RCP FILE: ${LOGP}${LOGF}" >> ${LOG}
}

#set pid lock
if [ ! -f ${LOCKFILE} ]
then
   echo $$ > ${LOCKFILE}
else
   echo "ERROR: Another instance of $BLOCK already running" >> ${LOG}
   exit 2
fi
# ==========================
echo "=================================" >> ${LOG}
echo "START: RCP of BLOCK ${BLOCK}" >> ${LOG}
for ip in 2 3 4
do
   let n=${ip}-1
   LOGF="${BLOCK}-${n}.LOG"
   log
   CMD_a="rcp 10.104.0.${ip}:${LOGP}${LOGF} ${BASEDIR}/${n}/${LOGF}.tmp"
   `$CMD_a`
   log
done

for ip in 2 3 4
do
   let n=${ip}-1
   LOGF="${BLOCK}-${n}.LOG"
   CMD_b="mv ${BASEDIR}/${n}/${LOGF}.tmp ${BASEDIR}/${n}/${LOGF}"
   `$CMD_b`
done

echo "END  : RCP of BLOCK ${BLOCK}" >> ${LOG}
echo "=================================" >> ${LOG}
# ==========================
# remove pid lock
rm ${LOCKFILE}
