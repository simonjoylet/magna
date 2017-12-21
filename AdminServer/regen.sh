test -f Makefile && make clean

rm -f *.h *.cc *.cpp Makefile *.conf

/usr/local/etc/phxrpc/codegen/phxrpc_pb2server -I /usr/local/include -I /usr/local/etc/phxrpc -f admin.proto -d .


