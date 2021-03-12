#include "shell.hh"

int main()
{ std::string input;

  // ToDo: Vervang onderstaande regel: Laad prompt uit bestand
  std::string prompt = "";

  int fd = syscall(SYS_open, "prompt.txt", O_RDONLY, 0755); // Gebruik de SYS_open call om een bestand te openen.
  char byte[1];                                           // 0755 zorgt dat het bestand de juiste rechten krijgt (leesbaar is).
  while(syscall(SYS_read, fd, byte, 1)){
      prompt.append(byte);
  }                   // Blijf SYS_read herhalen tot het bestand geheel gelezen is,       

  while(true)
  { std::cout << prompt << std::endl;                   // Print het prompt
    std::getline(std::cin, input);         // Lees een regel
    if (input == "new_file") new_file();   // Kies de functie
    else if (input == "ls") list();        //   op basis van
    else if (input == "src") src();        //   de invoer
    else if (input == "find") find();
    else if (input == "seek") seek();
    else if (input == "exit") return 0;
    else if (input == "quit") return 0;
    else if (input == "error") return 1;
    
    if (std::cin.eof()) return 0; } }      // EOF is een exit

void new_file() // ToDo: Implementeer volgens specificatie.
{
    std::string bestandsnaam; // bestandsnaam
    std::string tekst; // input van de gebruiker
    std::string temp_tekst; // tijdelijke input van gebruiker opslaan
    
    std::cout << "Voer een bestandsnaam in:" << std::endl; 
    std::getline(std::cin, bestandsnaam); // vraagt input van gebruiker

    const char* bestand[] = {bestandsnaam.c_str()}; // maakt een pointer om de bestandsnaam te lezen

    int fd = syscall(SYS_creat, bestand[0], 0755); // Create bestand en initialiseert file descriptor

    std::cout << "Voer een tekst in, sluit af met EOF: " << std::endl;

    while(std::getline(std::cin, tekst)){ // vraagt tekstuele input van gebruiker
        if (tekst == "<EOF>"){ // checkt of input niet EOF is
            break;
        }
        else{   // slaat input op in temp tekst
            temp_tekst += tekst + "\n";
        }
    }
    const char* line[] = {temp_tekst.c_str()}; // maakt een pointer van line
    syscall(SYS_write, fd, line[0], temp_tekst.size()); // write naar de fd vanaf begin van pointer tot eind temp tekst.
}

void list() // ToDo: Implementeer volgens specificatie.
{   
    int pid = syscall(SYS_fork); // initieert een fork en assignt return value naar pid
    if (pid == 0){  // als return value van pid 0 is, voert functie als child uit
        const char* arguments[] = {"/bin/ls", "-l", "-a", NULL};
        syscall(SYS_execve, arguments[0], arguments, NULL); 
    }
    else{ // als return value niet 0 is (error), wacht tot kind sterft
        syscall(SYS_wait4, pid, NULL, NULL, NULL);
    }
}

void find() // ToDo: Implementeer volgens specificatie.
{
    std::string zoekterm = ""; 
    std::cout << "Voer een zoekterm in:" << std::endl; 
    std::getline(std::cin, zoekterm); // vraagt input van gebruiker

    int file_descriptor[2];
    syscall(SYS_pipe, &file_descriptor);

    // Process 1 find
    int pid = syscall(SYS_fork); // initieert een fork en assignt return value naar pid
    if (pid == 0){  // als return value van pid 0 is, voert functie als child uit
        syscall(SYS_close, file_descriptor[0]);
        syscall(SYS_dup2, file_descriptor[1], 1);
        const char* arguments[] = {"/usr/bin/find", ".", NULL};
        syscall(SYS_execve, arguments[0], arguments, NULL); 
    }
    else{ // als return value niet 0 is (error), wacht tot kind sterft
        syscall(SYS_wait4, pid, NULL, NULL, NULL);
    }
    // Process 2 grep
    int pid_2 = syscall(SYS_fork); // initieert een fork en assignt return value naar pid
    if (pid_2 == 0){  // als return value van pid 0 is, voert functie als child uit
        syscall(SYS_close, file_descriptor[1]);
        syscall(SYS_dup2, file_descriptor[0], 0);
        const char* arguments_2[] = {"/bin/grep", zoekterm.c_str(), NULL};
        syscall(SYS_execve, arguments_2[0], arguments_2, NULL); 
    }
    else{ // als return value niet 0 is (error), wacht tot kind sterft
        syscall(SYS_close, file_descriptor[0]);
        syscall(SYS_close, file_descriptor[1]);
        syscall(SYS_wait4, pid_2, NULL, NULL, NULL);
    }
}

void seek() // ToDo: Implementeer volgens specificatie.
{
    int fd_1 = syscall(SYS_creat, "seek.txt", 0755); // maak seek.txt aan en assigned aan fd_1
    int fd_2 = syscall(SYS_creat, "loop.txt", 0755); // maak loop.txt aan en assigned aan fd_2
    char x[1] = {'x'}; //maakt een char aan met 'x' (1 char = 1 byte)
    char empty_byte[1] = {'\n'}; //maakt een char aan met '\n' (NULL)

    // Write seek.txt
    syscall(SYS_write, fd_1, x, 1); // vult een x in op de eerste plek
    syscall(SYS_lseek, fd_1, 5000001, 0); // zoekt naar positie 5000001 vanaf begin van het bestand
    syscall(SYS_write, fd_1, x, 1); // plaats nog een x op positie 5000001
    
    // Write loop.txt
    syscall(SYS_write, fd_2, x, 1); // vult een x in op de eerste plek
    for(unsigned int i = 0; i < 5000001; i++){ 
        syscall(SYS_write, fd_2, empty_byte, 1); // vult 5000000 keer een NULL in.
    syscall(SYS_write, fd_2, x, 1); // vult na de for loop nog een x in
    }
}

void src() // Voorbeeld: Gebruikt SYS_open en SYS_read om de source van de shell (shell.cc) te printen.
{ int fd = syscall(SYS_open, "shell.cc", O_RDONLY, 0755); // Gebruik de SYS_open call om een bestand te openen.
  char byte[1];                                           // 0755 zorgt dat het bestand de juiste rechten krijgt (leesbaar is).
  while(syscall(SYS_read, fd, byte, 1))                   // Blijf SYS_read herhalen tot het bestand geheel gelezen is,
    std::cout << byte; }                                  //   zet de gelezen byte in "byte" zodat deze geschreven kan worden.
