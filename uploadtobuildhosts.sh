LOCALDIR=/home/tobi/workspace_qt/ghostc
REMOTESERVER1=192.168.37.249
REMOTESERVER2=192.168.37.250
REMOTEPATH=/build/ghostc/src
LOGIN=admin
PASSWORD=mysecretpassword
 
cd $LOCALDIR
ftp -n $REMOTESERVER1 <<INPUT_END
quote user $LOGIN
quote pass $PASSWORD
cd $REMOTEPATH
prompt off
mdelete *
mput *
exit
INPUT_END

cd "$LOCALDIR"/smtpclient
ftp -n $REMOTESERVER1 <<INPUT_END
quote user $LOGIN
quote pass $PASSWORD
cd "$REMOTEPATH"/smtpclient
prompt off
mdelete *
mput *
exit
INPUT_END

cd $LOCALDIR
ftp -n $REMOTESERVER2 <<INPUT_END
quote user $LOGIN
quote pass $PASSWORD
cd $REMOTEPATH
prompt off
mdelete *
mput *
exit
INPUT_END

cd "$LOCALDIR"/smtpclient
ftp -n $REMOTESERVER2 <<INPUT_END
quote user $LOGIN
quote pass $PASSWORD
cd "$REMOTEPATH"/smtpclient
prompt off
mdelete *
mput *
exit
INPUT_END

exit 0
