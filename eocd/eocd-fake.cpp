extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include<malloc.h>
#include <getopt.h>
}

#include <devs/EOC_dummy1.h>
#include <db/EOC_loop.h>
#include <utils/EOC_ring_container.h>
#include <db/EOC_db.h>
#include <engine/EOC_engine.h>
#include <engine/EOC_engine_act.h>

#include <EOC_main.h>


/* Change this to whatever your daemon is called */
#define DAEMON_NAME "eocd"

/* Change this to the user under which to run */
#define RUN_AS_USER "artpol"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


EOC_main *m;

#define REPEATERS 2
dummy_channel CHANS[2][REPEATERS+1][2];
EOC_dummy1 *Nxx[2][REPEATERS][2];
EOC_engine *Ex[2][REPEATERS];
EOC_engine *E[2];
EOC_dummy1 *Nx[2][2];

EOC_dev_terminal *
init_dev(char *name_1)
{

    if( !strcmp(name_1,"dsl2") ){
	return Nx[0][0];
    }
    if( !strcmp(name_1,"dsl1") ){
	return Nx[1][0];
    }
/*
    if( !strcmp(name_1,"dsl2") ){
	printf("Init!!!! dsl2\n");
	return Nx[2][0];
    }
    if( !strcmp(name_1,"dsl3") ){
	return Nx[3][0];
    }
    if( !strcmp(name_1,"dsl4") ){
	return Nx[4][0];
    }
    if( !strcmp(name_1,"dsl5") ){
	return Nx[5][0];
    }
    if( !strcmp(name_1,"dsl6") ){
	return Nx[6][0];
    }
    if( !strcmp(name_1,"dsl7") ){
	return Nx[7][0];
    }
    if( !strcmp(name_1,"dsl8") ){
	return Nx[8][0];
    }
    if( !strcmp(name_1,"dsl9") ){
	return Nx[9][0];
    }
    if( !strcmp(name_1,"dsl10") ){
	return Nx[10][0];
    }
    if( !strcmp(name_1,"dsl11") ){
	return Nx[11][0];
    }
    if( !strcmp(name_1,"dsl12") ){
	return Nx[12][0];
    }
    if( !strcmp(name_1,"dsl13") ){
	return Nx[13][0];
    }
    if( !strcmp(name_1,"dsl14") ){
	return Nx[14][0];
    }
*/
    return NULL;
}



static void child_handler(int signum)
{
    switch(signum) {
    case SIGALRM:
	printf("Fail to start eocd daemon\n");
	exit(EXIT_FAILURE);
    case SIGUSR1:
	printf("Daemon started successfully\n");
        exit(EXIT_SUCCESS);
    case SIGCHLD:
	printf("Fail to start eocd daemon\n");
	exit(EXIT_FAILURE);
    }
}

static int
daemonize( )
{
    pid_t pid, sid, parent;
    int lfp = -1;

    // already a daemon
    if ( getppid() == 1 )
	return -1;

    // TODO: Perform locking to prevent double start

    // Drop user if there is one, and we were run as root
    if ( getuid() == 0 || geteuid() == 0 ) {
        struct passwd *pw = getpwnam(RUN_AS_USER);
        if ( pw ) {
            syslog( LOG_NOTICE, "setting user to " RUN_AS_USER );
            setuid( pw->pw_uid );
        }
    }

    // Trap signals that we expect to recieve
    signal(SIGCHLD,child_handler);
    signal(SIGUSR1,child_handler);
    signal(SIGALRM,child_handler);

    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        syslog( LOG_ERR, "unable to fork daemon, code=%d (%s)",
                errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }
    // If we got a good PID, then we can exit the parent process
    if (pid > 0) {
        // Wait for confirmation from the child via SIGTERM or SIGCHLD, or
        //   for two seconds to elapse (SIGALRM).  pause() should not return.
        alarm(2);
        pause();
	printf("Fail to start eocd server\n");
        exit(EXIT_FAILURE);
    }

    // At this point we are executing as the child process
    parent = getppid();

    // Cancel certain signals
    signal(SIGCHLD,SIG_DFL); // A child process dies
    signal(SIGTSTP,SIG_IGN); // Various TTY signals
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    signal(SIGHUP, SIG_IGN); // Ignore hangup signal
    signal(SIGTERM,SIG_DFL); // Die on SIGTERM

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        syslog( LOG_ERR, "unable to create a new session, code %d (%s)",
                errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    // Change the current working directory.  This prevents the current
    //   directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0) {
        syslog( LOG_ERR, "unable to change directory to %s, code %d (%s)",
                "/", errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    // Redirect standard files to /dev/null
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);

    // Tell the parent process that we are A-okay
    return parent;
}


void print_usage(char *name)
{
    printf("Usage: %s [-d] [-c <conf-file-path>] [-l <debug-level>]\n"
	    "Options:\n"
	    "  -d, --daemon\t\t\tRun as daemon\n"
	    "  -c, --config_path=<path>\tLocation of config file\n"
	    "  -l, --debuglev=<level>\tDebug level (0,1,2)\n",
	    name);
}

int main( int argc, char *argv[] ) {
    // Initialize the logging interface
    int need_daemonize = 0;
    int parent = -1;
    char config_path[256] = "/etc/eocd/eocd.conf";

    openlog( DAEMON_NAME, LOG_PID, LOG_LOCAL5 );
    syslog( LOG_INFO, "starting" );

    debug_lev = DOFF;

    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
    	    {"daemon", 0, 0, 'd'},
    	    {"config_path", 1, 0, 'c'},
    	    {"debuglev", 2, 0, 'l'},
    	    {"help", 2, 0, 'h'},
    	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "dl::c:",
                long_options, &option_index);
        if (c == -1)
    	    break;
	switch (c) {
        case 'd':
	    need_daemonize = 1;
            break;
        case 'c':
	    strncpy(config_path,optarg,256);
    	    break;
	case 'h':
	    print_usage(argv[0]);
	    return 0;
	case 'l':
	    printf("Debug level setting\n");
	    char c = '0';
	    if(optarg){
		c = optarg[0];
	    }
	    debug_lev = c - '0';
	    switch(debug_lev){
	    case 0:
		debug_lev = DERR;
		printf("Debug level: DEBUG ERRORS\n");
		break;
	    case 1:
		debug_lev = DINFO;
		printf("Debug level: DEBUG INFO\n");
		break;
	    case 2:
		debug_lev = DFULL;
		printf("Debug level: DEBUG FULL\n");
		break;
	    default:
		printf("unknown \"debuglev\" argument (%c), switch debug off\n",c);
		debug_lev = DOFF;
		continue;
	    }
	}
    }


    // Daemonize
    if( need_daemonize ){
	debug_lev = DOFF;
	parent = daemonize();
    }

    signal(SIGPIPE,SIG_IGN);


    /* Now we are a daemon -- do the work for which we were paid */

    for(int i=0;i<2;i++){
	Nx[i][0] = new EOC_dummy1("m-cs",&CHANS[i][0][0],&CHANS[i][0][1]);
	Nx[i][1] = new EOC_dummy1("s-ns",&CHANS[i][REPEATERS][1],&CHANS[i][REPEATERS][0]);
	E[i] = new EOC_engine(Nx[i][1],slave);
        for(int kk=0;kk<REPEATERS;kk++){
	    Nxx[i][kk][0] = new EOC_dummy1("R",&CHANS[i][kk][1],&CHANS[i][kk][0]);
	    Nxx[i][kk][1] = new EOC_dummy1("R",&CHANS[i][kk+1][0],&CHANS[i][kk+1][1]);
	    Ex[i][kk] = new EOC_engine(Nxx[i][kk][0],Nxx[i][kk][1]);
	}
    }


    m = new EOC_main(config_path,"/var/eocd/");

    if( !m->get_valid() ){
	delete m;
	return -1;
    }

    if( need_daemonize && parent>0 ){
	kill( parent, SIGUSR1 );
    }


    int k = 0;
    side_perf S;
    while(1){
	m->poll_channels();
	m->app_listen(1);
	for(int i=0;i<15;i++){
	    E[i]->schedule();
	    for(int kk=0;kk<REPEATERS;kk++)
	        Ex[i][kk]->schedule();
	}
    }

    /* Finish up */
    syslog( LOG_NOTICE, "terminated" );
    closelog();
    return 0;
}



