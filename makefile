SRC := ./src
BUILD := ./build
INCLUDE := ./include

# Où copier l'exécutable (~/.local/bin est dans mon $PATH)
BIN_PATH := ~/.local/bin


C_FLAGS := -I $(INCLUDE) -Wall -g
C_LIBS := -lfl

# parser.c et lexer.c sont générés par bison et flex.
C_FILES := $(wildcard $(SRC)/*.c) $(SRC)/parser.c $(SRC)/lexer.c
HEADERS := $(wildcard $(INCLUDE)/*.h)
OBJS := $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(C_FILES))


.PHONY: clean


arc: $(OBJS)
	gcc $(C_FLAGS) $^ -o $@ $(C_LIBS)
	cp arc $(BIN_PATH)


# Un fichier objet dépend de sa source mais également des headers.
# Ainsi si un header est modifié on recompile tous les objets pour être 
# sûrs que les changements soient bien appliqués partout
$(BUILD)/%.o: $(SRC)/%.c $(HEADERS)
	gcc $(C_FLAGS) -c $< -o $@ $(C_LIBS)


# Règles pour le parser et le lexer
$(SRC)/parser.c $(INCLUDE)/parser.h: $(SRC)/parser.y
	bison -d -o $(SRC)/parser.c $<
	mv $(SRC)/parser.h $(INCLUDE)


$(SRC)/lexer.c: $(SRC)/lexer.lex $(INCLUDE)/parser.h
	flex -o $@ $<


clean:
	rm ./build/* arc $(SRC)/lexer.c $(SRC)/parser.c $(INCLUDE)/parser.h
