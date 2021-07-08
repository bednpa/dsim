# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Nazev souboru: Makefile                                   #
# Autor: Pavel Bednar (xbedna73@stud.fit.vutbr.cz)          #
# Datum vytvoreni: 21.11.2020                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

MAIN_NAME=dsim
NAME2=dsimLib
PACKAGE=04_xbedna73_xvisco00
CC=g++
CFLAGS=-std=c++17 -Wall -Wextra -Werror -pedantic

all: clean $(MAIN_NAME)

$(MAIN_NAME):
	@$(CC) $(CFLAGS) $(MAIN_NAME).cpp $(NAME2).cpp -o $(MAIN_NAME)

run:
	@./$(MAIN_NAME)
pack:
	@zip $(PACKAGE).zip $(MAIN_NAME).cpp $(NAME2).cpp  $(NAME2).h doc.pdf Makefile

clean:
	@rm -rf $(MAIN_NAME)
