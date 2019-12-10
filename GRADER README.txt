1. The integral assembler portions of this project are located in the following files:
- ...\GridRunner\Client_VS\ClientAssembler\ClientAssembler\Display.cpp inside the DrawGrid() function
- ...\GridRunner\Client_VS\ClientAssembler\ClientAssembler\Display.cpp inside the SetDrawColor(bool, bool, bool, bool) function
- ...\GridRunner\Server_VS\Server\ServerAssembler\ServerCompute.cpp inside the NextGridPosition(int, int, int, int, int) function


2. Networking code is all in C++ for obvious reasons (would take too long in assembler)


3. IMPORTANT: All references (sources used) are on top of ServerAssembler.cpp and ClientAssembler.cpp files 
