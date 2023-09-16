A brief description of qc-rpc usage on the Super Computer Fugaku

0. Prerequisite

  Build Qulacs:

	https://github.com/qulacs/qulacs.git

  For most environments, the easiest way to build this system is to
issue ./script/build_gcc.sh under the cloned directory.
	   
1. Build

  At the source tree top directory, run the configure:

	$ ./configure --prefix=/whare/to/install \
	  --enable-qcrpc \
	  --enable-homeshare \
	  --with-qulacs=/whare/the/qulacs/insralled

  The --enable-homeshare option is for separating ~/.omrpc_regitry by
host architectures by augmenting the registry directory name by `uname
-i` output, like ~/.omrpc_registry.aarch64. It is helpful on Fugaku or
any other environments where the user home directories are shared
between different architecture machines.

2. Sample code

  The following is a sample code covering whole the qc-rpc APIs,
existig in QC_API directory main.c:

----------------------------------------------------------------
----------------------------------------------------------------

  Line 24, QC_InitRemote() is called to initilize OmniRPC layer. Call
this if you need qcsim run on an RPC server (w/ slurm.) The call must
be the first function call of the application.

  Line 27, QC_Init() is called to initialize QC_API layer. All the
application must call this function before using QC_APIs other than
QC_InitRemote().

  Line 37 and 42, QC_{Save|Load}Context() is used to save/restore the
qcsim application circuit configuration information to/from a
file. These are mainly used on an RPC server/slurm side.
  
  Line 39, QC_MeasureRemote() starts the simulation (measurement) with
given circuit configuratioon. The do_job_submit argiment controls how
the simuration runs via, '1' for via slurm, '0' for OmniRPC REX direct
execution.

  Line 46, contrary to QC_MeasureRemote(), QC_Measure() starts the
simuration on the local machine.

  Finally line 49, QC_Finalize() finalizes the QC_API layer.

3. Compiling applications

  All the needed headers are installed in ${prefix}/include, and all
the needed libraries are installed in ${prefix}/lib directory. Link
libqcs.so, libomrpc_client.so, and libomrpc_io.so.

e.g.
	LIBDIR	=	$(PREFIX)/lib
	CPPFLAGS	+=	-I$(PREFIX)/include
	LDFLAGS += -Wl,-rpath -Wl,$(LIBDIR) -L$(LIBDIR} \
		-lqcs -lomprpc_clinet -lomprpc_io



