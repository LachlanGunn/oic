#include <string.h>
#include <stdio.h>

#include "scpiparser.h"

scpi_error_t measure_callback(struct scpi_token* command)
{
	printf("Measured voltage!\n");
	
	return SCPI_SUCCESS;
}

void print_command_tree(struct scpi_command* list, int tabs)
{
	while(list != NULL)
	{
		int i;
		
		for(i = 0; i < tabs; i++)
		{
			putchar('\t');
		}
		fwrite(list->long_name, 1, list->long_name_length, stdout);
		putchar('\n');
		print_command_tree(list->children, tabs+1);
		
		list = list->next;
	}
}

void print_single_command(struct scpi_command* list)
{
	fwrite(list->long_name, 1, list->long_name_length, stdout);
	putchar('\n');
	print_command_tree(list->children, 1);
}

int main(int argc, char** argv)
{
	char* test_string = ":MEASURE:VOLTAGE CH1,CH2";
	struct scpi_token* tokens;
	struct scpi_token* current_token;
	
	struct scpi_command command_tree;
	struct scpi_command* measure;
	struct scpi_command* found_command;
	
	tokens = scpi_parse_string(test_string, strlen(test_string));
	printf("Parsing string: %s\n\n", test_string);
	
	for(current_token = tokens; current_token != NULL; current_token = current_token->next)
	{
		printf("%d\t", current_token->type);
		fwrite(current_token->value, 1, current_token->length, stdout);
		printf("\n");
	}
	
	printf("\nAssembling command tree:\n\n");
	
	command_tree.next = NULL;
	command_tree.children = NULL;
	
	command_tree.long_name = NULL;
	command_tree.long_name_length = 0;
	
	command_tree.short_name = NULL;
	command_tree.short_name_length = 0;
	
	command_tree.callback = NULL;
	
	measure = scpi_register_command(&command_tree, "MEASURE", 7, "MEAS", 4, NULL);
	
	scpi_register_command(measure, "VOLTAGE", 7, "VOLT", 4, measure_callback);
	scpi_register_command(measure, "FREQUENCY", 9, "FREQ", 4, NULL);
	
	print_command_tree(&command_tree, 0);
	
	printf("\nFinding :MEASURE:VOLTAGE\n\n");
	
	found_command = scpi_find_command(&command_tree, tokens);
	
	if(found_command == NULL)
	{
		printf("Could not find command.\n");
	}
	else
	{
		print_single_command(found_command);
	}
	
	printf("\n\nExecuting :MEASURE:VOLTAGE\n");
	scpi_execute_command(&command_tree, ":MEASURE:VOLTAGE", strlen(":MEASURE:VOLTAGE"));
	
	return 0;
}