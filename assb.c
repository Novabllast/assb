//-----------------------------------------------------------------------------
// assb.c
//
// C program which interprets and debugs a Turing machine.
//
// Group: 13492 study assistant Angela Promitzer
//
// Authors: Manfred Böck 1530598, Patrick Struger 1530664, Anna Haupt 1432018
//
// Latest Changes: 10.02.2016 (by Patrick Struger)
//-----------------------------------------------------------------------------
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum _Boolean_
{
  FALSE = 0,
  TRUE = 1
} Boolean;

typedef struct _Breakpoint_
{
  char value_;
  char* type_;
} Breakpoint;

typedef struct _Rules_
{
  int current_state_;
  char readed_symbol_;
  char symbol_to_write_;
  int next_state_;
  char head_movement_;
} Rules;

typedef struct _Turing_
{
  char* band_;
  int head_position_;
  int start_state_;
  int current_rule_;
  int current_state_;
  int rules_count_;
  Rules* rules_;
  Breakpoint* breakpoints_;
  int breakpoint_counter_;
  Boolean turing_over_;
} Turing;

#define RULE_PARAMETER_COUNT 5

#define EVERYTHING_WORKED_FINE 0
#define ERROR_CODE_WRONG_PARAMETER 1
#define ERROR_CODE_OUT_OF_MEMORY 2
#define ERROR_CODE_PARSING_THE_FILE_FAILED 3
#define ERROR_CODE_READING_THE_FILE_FAILED 4
#define ERROR_CODE_NONE_DETERMINISTIC_MACHINE 5

#define OUT_OF_MEMORY "[ERR] out of memory\n"
#define WRONG_PARAMETER_COUNT "[ERR] usage: ./assb <file>\n"
#define PARSING_THE_FILE_FAILED "[ERR] parsing of input failed\n"
#define READING_THE_FILE_FAILED "[ERR] reading the file failed\n"
#define NONE_DETERMINISTIC_MACHINE "[ERR] non-deterministic turing machine\n"

Boolean isPlusOrMinus(char character);
Boolean checkIfBandIsEmpty(Turing* machine);
Boolean checkIntBreakpoints(Turing*, char*, int);
Boolean checkCharBreakpoints(Turing*, char*, char);

int checkDeterministic(Turing* machine);
int interactiveDebugMode(Turing* machine);
int checkMemoryAvailable(char** array, int size);
int loadTextFile(char* filename, Turing* machine);
int setBreakPoint(Turing* machine, char* type, char value);
int handleUserInput(char* action, char* delimiter, Turing* machine);

void list(Turing* machine);
void show(Turing* machine);
void step(Turing* machine);
void executeRules(Turing* machine);
void freeMemory(Turing* machine, char*, int);

int main(int argc, char *argv[])
{
  int return_value = EVERYTHING_WORKED_FINE;

  if (argc == 2)
  {
    Turing machine = {NULL, 0, 0, 0, 0, 0, NULL, NULL, 0, FALSE};

    machine.band_ = malloc(50 * sizeof(char));
    if (!machine.band_)
      freeMemory(&machine, OUT_OF_MEMORY, ERROR_CODE_OUT_OF_MEMORY);

    memset(machine.band_, '_', 49);
    machine.band_[49] = '\0';

    machine.rules_ = calloc(50, sizeof(Rules));
    if (!machine.rules_)
      freeMemory(&machine, OUT_OF_MEMORY, ERROR_CODE_OUT_OF_MEMORY);

    machine.breakpoints_ = calloc(50, sizeof(Breakpoint));
    if (!machine.breakpoints_)
      freeMemory(&machine, OUT_OF_MEMORY, ERROR_CODE_OUT_OF_MEMORY);

    return_value = loadTextFile(argv[1], &machine);
    if (return_value == EVERYTHING_WORKED_FINE)
      interactiveDebugMode(&machine);

    int free_counter = 0;
    for (; free_counter < machine.breakpoint_counter_; free_counter++)
    {
      free(machine.breakpoints_[free_counter].type_);
      machine.breakpoints_[free_counter].type_ = NULL;
    }

    free(machine.band_);
    machine.band_ = NULL;
    free(machine.rules_);
    machine.rules_ = NULL;
    free(machine.breakpoints_);
    machine.breakpoints_ = NULL;
  }
  else
  {
    printf(WRONG_PARAMETER_COUNT);
    return_value = ERROR_CODE_WRONG_PARAMETER;
  }
  return return_value;
}

//-----------------------------------------------------------------------------
///
/// //TODO
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @param message //TODO
/// @param exit_code //TODO
//
void freeMemory(Turing* machine, char* message, int exit_code)
{
  free(machine->band_);
  machine->band_ = NULL;
  free(machine->rules_);
  machine->rules_ = NULL;
  free(machine->breakpoints_);
  machine->breakpoints_ = NULL;
  printf("%s", message);
  exit(exit_code);
}

//-----------------------------------------------------------------------------
///
/// Check if memory for realloc is available.
///
/// @param array The array which should be reallocated
/// @param size The new size of the array which should be reallocated
/// @return int (2) - out of memory
///         int (0) - array successfully reallocated
//
int checkMemoryAvailable(char** array, int size)
{
  char* temporary_pointer = NULL;
  temporary_pointer = realloc(*array, size * sizeof(char));
  if(!temporary_pointer)
  {
    free(*array);
    *array = NULL;
    printf(OUT_OF_MEMORY);
    return ERROR_CODE_OUT_OF_MEMORY;
  }
  else
    *array = temporary_pointer;

  return EVERYTHING_WORKED_FINE;
}

//-----------------------------------------------------------------------------
///
/// Loads the given file.
///
/// @param filename filename The Path of the file which should be loaded.
/// @return int (2) - out of memory
///         int (4) - reading the file failed
///         int (0) - file successfully loaded
//
int loadTextFile(char* filename, Turing* machine)
{
  int next_state = 0;
  int rules_limit = 50;
  int rules_counter = 0;
  int character_limit = 60;
  int current_position = 0;
  int character_counter = 0;
  int return_value = EVERYTHING_WORKED_FINE;
  int rules_parameter = RULE_PARAMETER_COUNT;

  char character = 0;
  char write_symbol = 0;
  char readed_symbol = 0;
  char head_movement = 0;

  char* start_state = calloc(60, sizeof(char));
  char* head_position = calloc(65, sizeof(char));
  
  if (!start_state || !head_position)
  {
    free(start_state);
    free(head_position);
    start_state = NULL;
    head_position = NULL;
    freeMemory(machine, OUT_OF_MEMORY, ERROR_CODE_OUT_OF_MEMORY);
  }

  FILE* file_to_read = fopen(filename, "r");
  if (file_to_read)
  {
    fscanf(file_to_read, "%s\n", machine->band_);
    //read aisPlusOrMinus(character)nd parse head_position
    while ((character = fgetc(file_to_read)) != '\n' && character != EOF)
    {
      if (!character_counter == 0 || !isPlusOrMinus(character))
        if (!isdigit(character))
        {
          free(head_position);
          free(start_state);
          start_state = NULL;
          head_position = NULL;
          fclose(file_to_read);
          freeMemory(machine,
                   PARSING_THE_FILE_FAILED, ERROR_CODE_PARSING_THE_FILE_FAILED);
        }

      head_position[character_counter] = character;
      character_counter++;
      if (character_counter >= character_limit)
      {
        character_limit *= 2;
        //Realloc if longer than character_limit
        return_value = checkMemoryAvailable(&head_position,
                                           (character_limit + 1));
        if (return_value == ERROR_CODE_OUT_OF_MEMORY)
        {
          fclose(file_to_read);
          return ERROR_CODE_OUT_OF_MEMORY;
        }
      }
    }
    machine->head_position_ = strtol(head_position, NULL, 10);
    free(head_position);
    head_position = NULL;

    //read an parse start_state_
    character_limit = 60;
    character_counter = 0;
    
    while ((character = fgetc(file_to_read)) != '\n' && character != EOF)
    {
      if (!character_counter == 0 || !isPlusOrMinus(character))
        if (!isdigit(character))
        {
          free(start_state);
          start_state = NULL;
          fclose(file_to_read);
          freeMemory(machine,
                   PARSING_THE_FILE_FAILED, ERROR_CODE_PARSING_THE_FILE_FAILED);
        }
      start_state[character_counter] = character;
      character_counter++;
      if (character_counter >= character_limit)
      {
        character_limit *= 2;
        //Realloc if longer than character_limit
        return_value = checkMemoryAvailable(&start_state,
                                           (character_limit + 1));
        if (return_value == ERROR_CODE_OUT_OF_MEMORY)
        {
          fclose(file_to_read);
          return ERROR_CODE_OUT_OF_MEMORY;
        }
      }
    }
    machine->start_state_ = strtol(start_state, NULL, 10);
    machine->current_state_ = machine->start_state_;
    free(start_state);
    start_state = NULL;

    while (rules_parameter == RULE_PARAMETER_COUNT)
    {
      rules_parameter = fscanf(file_to_read, "%d %c %c %d %c\n",
							   &current_position, &readed_symbol,
							   &write_symbol, &next_state, &head_movement);
      if (rules_parameter != EOF && rules_parameter != RULE_PARAMETER_COUNT)
      {
        printf(PARSING_THE_FILE_FAILED);
        return_value = ERROR_CODE_PARSING_THE_FILE_FAILED;
      }
      else
      {//TODOD reallocate if rulescounter größer rulesmax
        machine->rules_[rules_counter].current_state_ = current_position;
        machine->rules_[rules_counter].readed_symbol_ = readed_symbol;
        machine->rules_[rules_counter].symbol_to_write_ = write_symbol;
        machine->rules_[rules_counter].next_state_ = next_state;
        machine->rules_[rules_counter].head_movement_ = head_movement;
      }
      rules_counter++;
    }
    machine->rules_count_ = rules_counter - 1;

    fclose(file_to_read);
    if (return_value == EVERYTHING_WORKED_FINE)
      return_value = checkDeterministic(machine);
  }
  else
  {
    printf(READING_THE_FILE_FAILED);
    return_value = ERROR_CODE_READING_THE_FILE_FAILED;
  }

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to check wether the rules for the turing machine
/// are deterministic or not
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @return int (0) - no errors
///         int (5) - none deterministic machine
//
int checkDeterministic(Turing* machine)
{
  int state = 0;
  char symbol = 0;
  
  int rules_counter = 0;
  int second_counter = 1;
  int rules_max = machine->rules_count_;

  while (rules_counter < rules_max)
  {
    state = machine->rules_[rules_counter].current_state_;
    symbol = machine->rules_[rules_counter].readed_symbol_;
    second_counter = rules_counter + 1;
    for (; second_counter < rules_max; second_counter++)
    {
      if (state == machine->rules_[second_counter].current_state_ &&
          symbol == machine->rules_[second_counter].readed_symbol_)
      {
        printf(NONE_DETERMINISTIC_MACHINE);
        return ERROR_CODE_NONE_DETERMINISTIC_MACHINE;
      }
    }
    rules_counter++;
  }
  return EVERYTHING_WORKED_FINE;
}

//-----------------------------------------------------------------------------
///
/// Function to start interactive debug
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @return int (0) - no errors
///         int (2) - out of memory
//
int interactiveDebugMode(Turing* machine)
{
  char character = 0;
  Boolean close_program = FALSE;

  int action_input_limit = 5;
  int action_input_counter = 0;
  int return_value = EVERYTHING_WORKED_FINE;

  char* action = NULL;
  char* delimiter = " ";
  char* user_input = calloc(55, sizeof(char));

  while (!close_program)
  {
    printf("esp> ");
    while((character = getchar()) != '\n' && character != EOF)
    {
      user_input[action_input_counter] = character;
      action_input_counter++;
      if (action_input_counter >= action_input_limit)
      {
        action_input_limit *= 2;
        return_value = checkMemoryAvailable(&user_input,
                                           (action_input_limit + 1));
        if (return_value == ERROR_CODE_OUT_OF_MEMORY)
          return return_value;
      }
    }

    //check if there is a user input
    if (user_input[0] != 0  && user_input)
    {
      user_input[action_input_counter] = '\0';
      action = strtok(user_input, delimiter);
      if (strcmp(action, "quit") == 0 || character == EOF || feof(stdin))
      {
        printf("Bye.\n");
        close_program = TRUE;
      }
      else
      {
        return_value = handleUserInput(action, delimiter, machine);
      }
    }

    //Resets the user input string to the size of 50
    action_input_counter = 0;
    return_value = checkMemoryAvailable(&user_input, 50);
    if (return_value == ERROR_CODE_OUT_OF_MEMORY)
      return return_value;
    user_input[49] = '\0';

    if (machine->turing_over_)
    {
      close_program = TRUE;
      printf("machine stopped in state %i\n", machine->current_state_);
      show(machine);
    }
  }
  free(user_input);
  user_input = NULL;

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to reacting to user inputs.
///
/// @param action //TODO
/// @param delimiter //TODO
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @return int (0) - no errors
///         int (2) - out of memory
//
int handleUserInput(char* action, char* delimiter, Turing* machine)
{
  int return_value = EVERYTHING_WORKED_FINE;

  if (strcmp(action, "list") == 0)
  {
    list(machine);
  }
  else if (strcmp(action, "step") == 0)
  {
    step(machine);
  }
  else if (strcmp(action, "show") == 0)
  {
    show(machine);
  }
  else if (strcmp(action, "continue") == 0)
  {
    executeRules(machine);
  }
  else if (strcmp(action, "break") == 0)
  {
    char* breakpoint_type = NULL;
    char* second_parameter = NULL;
    breakpoint_type = strtok(NULL, delimiter);
    second_parameter = strtok(NULL, delimiter);

    if (breakpoint_type && second_parameter)
    {
      char breakpoint_value = second_parameter[0];
      return_value = setBreakPoint(machine, breakpoint_type, breakpoint_value);
    }
  }

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to display all rules of the Turingmachine.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
//
void list(Turing* machine)
{
  int jump = -1;
  int state = 0;
  int next_state = 0;
  int rules_counter = 0;
  int rules_count = machine->rules_count_;
  int head_position = machine->head_position_;

  char symbol = 0;
  char head_movement = 0;
  char symbol_to_write = 0;
  Boolean FOUND_RULE = FALSE;
  
  for (; rules_counter < rules_count; rules_counter++)
  {
    symbol = machine->rules_[rules_counter].readed_symbol_;
    state = machine->rules_[rules_counter].current_state_;
    next_state = machine->rules_[rules_counter].next_state_;
    head_movement = machine->rules_[rules_counter].head_movement_;
    symbol_to_write = machine->rules_[rules_counter].symbol_to_write_;
    
    if (symbol == machine->band_[head_position] &&
        state == machine->current_state_ && !FOUND_RULE)
    {
      printf(">>> ");
      FOUND_RULE = TRUE;
      jump = rules_counter;
      rules_counter = -1;
    }
    
    if (FOUND_RULE && rules_counter != jump)
      printf("%i %c -> %c %i %c \n",
             state, symbol, symbol_to_write, next_state, head_movement);
    
    //if no rule matches then start from the beginning without showing next rule
    if (rules_counter == (rules_count - 1) && !FOUND_RULE)
    {
      rules_counter = -1;
      FOUND_RULE = TRUE;
    }
  }
}

//-----------------------------------------------------------------------------
///
/// Function to display the band of the Turingmachine.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
//
void show(Turing* machine)
{
  if (!checkIfBandIsEmpty(machine))
  {
    char symbol = 0;
    int band_position = 0;
    int band_length = strlen(machine->band_);

    if (machine->head_position_ < 0)
      band_position = machine->head_position_;
    else if (machine->head_position_ > band_length)
      band_length = machine->head_position_ + 1;

    for (; band_position < band_length; band_position++)
    {
      if (band_position < 0 || band_position > (strlen(machine->band_) - 1))
      {
        symbol = '_';
      }
      else
      {
        symbol = machine->band_[band_position];
      }
      if (band_position == machine->head_position_)
      {
        printf(">%c<", symbol);
      }
      else
      {
        printf("%c", symbol);
      }
      if (band_position < band_length - 1)
      {
        printf("|");
      }
    }
  }
  else
    printf(">_<");

  printf("\n");
}

//-----------------------------------------------------------------------------
///
/// Function to check if the band of the turing machine is empty.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @return Boolean (TRUE) - band is empty
///         Boolean (FALSE) - band is not empty
//
Boolean checkIfBandIsEmpty(Turing* machine)
{
  int band_position = 0;
  Boolean is_empty = FALSE;
  int band_length = strlen(machine->band_);

  for (; band_position < band_length; band_position++)
    if (machine->band_[band_position] == '_')
      is_empty = TRUE;

  return is_empty;
}

//-----------------------------------------------------------------------------
///
/// Function to execute the next matching rule and display the executed rule.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
//
void step(Turing* machine)
{
  int next_state = 0;
  int rules_counter = 0;
  int current_state = 0;
  int state_at_header_position = machine->current_state_;
  int head_position = machine->head_position_;

  char symbol = 0;
  char head_movement = 0;
  char symbol_to_read = 0;
  char symbol_to_write = 0;

  Boolean FOUND_RULE = FALSE;

  for (; rules_counter < machine->rules_count_ && !FOUND_RULE; rules_counter++)
  {
    current_state = machine->rules_[rules_counter].current_state_;
    if (current_state == state_at_header_position)
    {
      symbol = machine->band_[head_position];
      symbol_to_read = machine->rules_[rules_counter].readed_symbol_;
      if (symbol == symbol_to_read)
      {
        FOUND_RULE = TRUE;
        machine->current_rule_ = rules_counter;
        next_state = machine->rules_[rules_counter].next_state_;
        head_movement = machine->rules_[rules_counter].head_movement_;
        symbol_to_write = machine->rules_[rules_counter].symbol_to_write_;

        machine->current_state_ = next_state;
        machine->band_[head_position] = symbol_to_write;
        if (head_movement == 'R')
        {
          machine->head_position_++;
        }
        else if (head_movement == 'L')
        {
          machine->head_position_--;
        }
      }
    }
  }

  if (FOUND_RULE)
    printf("%i %c -> %c %i %c \n", current_state, symbol_to_read,
           symbol_to_write, next_state, head_movement);
  else
    machine->turing_over_ = TRUE;
}

//-----------------------------------------------------------------------------
///
/// Function to execute as many rules as possible till the next break point or
/// the end of the program (which means no rule is matching anymore).
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
//
void executeRules(Turing* machine) //TODO Write Breakpoint past noch nit
{
  int next_state = 0;
  int current_state = 0;
  int rules_counter = 0;
  int state_at_header_position = 0;
  int head_position = machine->head_position_;

  char symbol = 0;
  char head_movement = 0;
  char symbol_to_read = 0;
  char symbol_to_write = 0;

  for (; rules_counter < machine->rules_count_; rules_counter++)
  {
    if (checkIntBreakpoints(machine, "pos", head_position))
      break;

    state_at_header_position = machine->current_state_;
    if (checkIntBreakpoints(machine, "state", state_at_header_position))
      break;

    current_state = machine->rules_[rules_counter].current_state_;
    symbol = machine->band_[head_position];
    if (checkCharBreakpoints(machine, "read", symbol))
      break;

	symbol_to_write = machine->rules_[rules_counter].symbol_to_write_;
	if (checkCharBreakpoints(machine, "write", symbol_to_write))
	  break;

    if (current_state == state_at_header_position)
    {
      symbol_to_read = machine->rules_[rules_counter].readed_symbol_;
      if (symbol == symbol_to_read)
      {
        machine->current_rule_ = rules_counter;
        next_state = machine->rules_[rules_counter].next_state_;
        head_movement = machine->rules_[rules_counter].head_movement_;
        machine->current_state_ = next_state;
        machine->band_[head_position] = symbol_to_write;
        if (head_movement == 'R')
        {
          machine->head_position_++;
        }
        else if (head_movement == 'L')
        {
          machine->head_position_--;
        }
        head_position = machine->head_position_;
        rules_counter = -1;
      }
    }
  }

  if (rules_counter == machine->rules_count_)
    machine->turing_over_ = TRUE;
}

//-----------------------------------------------------------------------------
///
/// Function to check int Breakpoints.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @param type Type of the Breakpoint to be check.
/// @param value Value to be check.
/// @return Boolean (TRUE) - checkpoint exists
///         Boolean (FALSE) - checkpoint doesn't exist
//
Boolean checkIntBreakpoints(Turing* machine, char* type, int value)
{
  Boolean return_value = FALSE;

  int breakpoints_counter = 0;
  int breakpoints_length = machine->breakpoint_counter_;

  for (; breakpoints_counter < breakpoints_length; breakpoints_counter++)
  {
    if (strcmp(machine->breakpoints_[breakpoints_counter].type_, type) == 0)
      if (machine->breakpoints_[breakpoints_counter].value_ == value)
      {
        return_value = TRUE;
        machine->breakpoints_[breakpoints_counter].value_ = -1;
      }
  }

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to check char Breakpoints.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @param type Type of the Breakpoint to be check.
/// @param symbol Symbol to be check.
/// @return Boolean (TRUE) - checkpoint exists
///         Boolean (FALSE) - checkpoint doesn't exist
//
Boolean checkCharBreakpoints(Turing* machine, char* type, char symbol)
{
  Boolean return_value = FALSE;

  int breakpoints_counter = 0;
  int breakpoints_length = machine->breakpoint_counter_;

  for (; breakpoints_counter < breakpoints_length; breakpoints_counter++)
    if (strcmp(machine->breakpoints_[breakpoints_counter].type_, type) == 0)
      if (machine->breakpoints_[breakpoints_counter].value_ == symbol)
      {
        return_value = TRUE;
        machine->breakpoints_[breakpoints_counter].value_ = -1;
      }

  return return_value;
}

//-----------------------------------------------------------------------------
///
/// Function to set a Breakpoint.
///
/// @param machine Struct with all components to descripe the turing machine
///        (state, rules, etc.)
/// @param type Type of the Breakpoint to be set.
/// @param value Value of the Breakpoint to be set.
//
int setBreakPoint(Turing* machine, char* type, char value) //TODO Reallocate if breakpoints > Breakpointslimit
{
  if (type && value)
  {
    int int_value = 0;
    Boolean is_valide_type = FALSE;

    if (strcmp(type, "write") == 0 || strcmp(type, "read") == 0)
      is_valide_type = TRUE;
    else if (strcmp(type, "state") == 0 || strcmp(type, "pos") == 0)
    {
      is_valide_type = TRUE;
      int_value = value - '0';
    }

    if (is_valide_type)
    {
      int breakpoint_pos = machine->breakpoint_counter_;
      machine->breakpoint_counter_++;
      machine->breakpoints_[breakpoint_pos].type_ = calloc(6, sizeof(char));
      if(!(machine->breakpoints_[breakpoint_pos].type_))
      {
        free(machine->breakpoints_[breakpoint_pos].type_);
        machine->breakpoints_[breakpoint_pos].type_ = NULL;
        printf(OUT_OF_MEMORY);
        return ERROR_CODE_OUT_OF_MEMORY;
      }

      sprintf(machine->breakpoints_[breakpoint_pos].type_, "%s", type);
      if (strcmp(type, "state") == 0 || strcmp(type, "pos") == 0)
        machine->breakpoints_[breakpoint_pos].value_ = int_value;
      else
        machine->breakpoints_[breakpoint_pos].value_ = value;
    }
  }

  return EVERYTHING_WORKED_FINE;
}

//-----------------------------------------------------------------------------
///
/// Function to check whether the given character is a plus or a minus.
///
/// @param character The Character to be checked.
/// @return Boolean (TRUE) - is a plus or a minus
///         Boolean (FALSE) - is not a plus or a minus
//
Boolean isPlusOrMinus(char character)
{
  Boolean isPlusOrMinue = FALSE;
	if (character == '-' || character == '+')
      isPlusOrMinue = TRUE;

	return isPlusOrMinue;
}
