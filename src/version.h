#define NB_VERSION "1.0.0"
#define NB_REPO "mkalmousli/nanoblue"
#define NB_HANDLE "mkalmousli"
extern char upd_msg[96]; /* status of the background update check, empty when nothing to say */
int cli(int argc, char **argv); /* --version --update --uninstall --help; -1 = start the GUI */
void upd_start(void);           /* background check, installs a newer release in place unless NANOBLUE_NO_UPDATE is set */
