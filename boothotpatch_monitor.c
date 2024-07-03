#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


#define DEVICE_PATH "/dev/boothotpatch_device"



#define MAX_LINES 10
#define MAX_LENGTH 200

struct HotPatchAppInfo {
    char hotpatch_app_name[MAX_LENGTH];
    char hotpatch_app_path[MAX_LENGTH];
    char hotpatch_lib_path[MAX_LENGTH];
};

struct HotPatchAppInfo hotpatchapp[MAX_LINES];
pid_t monitor_pid;
char current_app[256];


void signal_handler(int signum)
{
	FILE *fp;
	int dev_fd;
	int i;
	
	fp = fopen("/home/gejingquan/projects/BootHotPatch/boothotpatch_monitor.txt", "a");	
    fprintf(fp,"Received signal %d\n", signum);

	dev_fd = open(DEVICE_PATH, O_RDONLY);
	if (dev_fd < 0) {
		fprintf(fp,"Failed to open device");
	}
	if (read(dev_fd, current_app, sizeof(current_app)) < 0) {
		fprintf(fp,"Failed to read current app name.");
	}
	close(dev_fd);
	fprintf(fp,"Current App name is %s\n", current_app);		
		
	for(i = 0; i < 10; i++){

		fprintf(fp,"boothotpatch: current_app: %s\n", current_app);
		fprintf(fp,"boothotpatch: hotpatchapp[%d].hotpatch_app_name: %s\n", 
			i,hotpatchapp[i].hotpatch_app_name);
		fprintf(fp,"strcmp(hotpatchapp[%d].hotpatch_app_name, current_app) == %d\n", 
			i,strcmp(hotpatchapp[i].hotpatch_app_name, current_app));
		
		if (strcmp(hotpatchapp[i].hotpatch_app_name, current_app) == 0) {
			fprintf(fp, "I find the correct HotPatchApp Name is: %s\n", hotpatchapp[i].hotpatch_app_name);
			break;
		}
	} 	

	

	sleep(5);
    char command[600];
    snprintf(command, sizeof(command), 
		" libcare-ctl -v patch -p $(pidof %s) %s", 
		hotpatchapp[i].hotpatch_app_name,
		hotpatchapp[i].hotpatch_lib_path);
    system(command);		
	i = 0;
	fclose(fp);

}
  
 
int main()
{
	int dev_fd;
	int ret;
	char line[MAX_LENGTH];
	FILE *fp;
	FILE *list_fp;	
	int count = 0;

	
	signal(SIGUSR1, signal_handler);	
	fp = fopen("/home/gejingquan/projects/BootHotPatch/boothotpatch_monitor.txt", "a");	
	monitor_pid = getpid();
	fprintf(fp,"My PID is: %d\n", monitor_pid);
	


	list_fp=fopen("/home/gejingquan/projects/BootHotPatch/boothotpatch_list.txt", "r");
    while (fgets(line, sizeof(line), list_fp) != NULL) {
        sscanf(line, "%s %s %s", hotpatchapp[count].hotpatch_app_name, 
			hotpatchapp[count].hotpatch_app_path, hotpatchapp[count].hotpatch_lib_path);
        count++;
        if (count >= MAX_LINES) {
            fprintf(fp, "Too many lines in the file.\n");
            break;
        }
    }	
    for (int i = 0; i < count; i++) {		
        fprintf(fp, "HotPatchApp Name: %s\n", hotpatchapp[i].hotpatch_app_name);
        fprintf(fp, "HotPatchApp Path: %s\n", hotpatchapp[i].hotpatch_app_path);
        fprintf(fp, "HotPatch Lib Path: %s\n", hotpatchapp[i].hotpatch_lib_path);
    }




	dev_fd = open(DEVICE_PATH, O_WRONLY);
	if (dev_fd < 0) {
		fprintf(fp,"Failed to open device");
		return EXIT_FAILURE;
	}
	if (write(dev_fd, &monitor_pid, sizeof(monitor_pid)) < 0) {
		fprintf(fp,"Failed to write PID to device");
		close(dev_fd);
		return EXIT_FAILURE;
	}
	fprintf(fp,"PID %d written to device\n", monitor_pid);
	ret = ioctl(dev_fd, 0, &hotpatchapp);
    if (ret < 0) {
        perror("Failed to send ioctl command");
        close(dev_fd);
        return -1;
    }	
	close(dev_fd);
	fclose(fp);

	while(1){
		fp = fopen("/home/gejingquan/projects/BootHotPatch/boothotpatch_monitor.txt", "a");
		fprintf(fp,"I'm monitoring all applications that require hot patching.\n"); 
        sleep(100); 
		fclose(fp);
	}
	
	return EXIT_SUCCESS;
}


