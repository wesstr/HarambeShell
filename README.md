# The amazing harambe shell
* Written for a homework assigment in OS
* Has manny issues including a massive memory leak
* Use only for a general understanding of writing a shell in Linux

# Features
* File io redirect for inputing commands into a file
    * ">, <, >>, <<"
* Single pipeing
    * "|"
* Alaising a command to another
    * Alises are stored in .alias.dat file
    * See code for notes on proper syntax in this file
* Writes a log file and stores entered commands
    * Logs are stored in the file "audit.log"
