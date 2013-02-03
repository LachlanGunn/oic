#include <string.h>
#include <stdio.h>

#include "scpiparser.h"

float voltage;
int   voltage_on;

scpi_error_t measure_callback(struct scpi_token* command)
{
	printf("%e\n", voltage_on ? voltage : 0.0f);
	scpi_free_tokens(command);
	
	return SCPI_SUCCESS;
}

scpi_error_t set_voltage(struct scpi_token* command)
{
	struct scpi_token* args;
	
	args = command;
	while(args != NULL && args->type == 0)
	{
		args = args->next;
	}
	
	if(args == NULL)
	{
		printf("ERROR: No argument.\n");
	}
	else
	{
		voltage = scpi_parse_numeric(args->value, args->length);
	}
	
	scpi_free_tokens(args);
	
	return SCPI_SUCCESS;
}

scpi_error_t set_output(struct scpi_token* command)
{
	struct scpi_token* args;
	
	args = command;
	while(args != NULL && args->type == 0)
	{
		args = args->next;
	}
	
	if(args == NULL)
	{
		printf("ERROR: No argument.\n");
	}
	else
	{
		if(args->length == 2 && args->value[0] == 'O' && args->value[1] == 'N')
		{
			voltage_on = 1;
		}
		else if(args->length == 3 &&
			args->value[0] == 'O' && args->value[1] == 'F' && args->value[2] == 'F')
		{
			voltage_on = 0;
		}
		else
		{
			voltage_on = (int)(0.5+scpi_parse_numeric(args->value, args->length)) ? 1 : 0;
		}
		
	}
	
	return SCPI_SUCCESS;
}

scpi_error_t get_output(struct scpi_token* command)
{
	printf("%d\n", voltage_on);
	return SCPI_SUCCESS;
}

void status_message(char* str)
{
	printf("[ %s ]\n", str);
}

void execute_command(struct scpi_command* root, char* str)
{
	printf(">> %s\n", str);
	scpi_execute_command(root, str, strlen(str));
	putchar('\n');
	/*
	printf("<< Error code: %d\n", scpi_execute_command(root, str, strlen(str)));
	*/
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
	
	struct scpi_command command_tree;
	struct scpi_command* measure;
	struct scpi_command* source;
	struct scpi_command* output;
	
	voltage = 0;
	voltage_on = 0;
	
	printf("\nAssembling command tree:\n\n");
	
	command_tree.next = NULL;
	command_tree.children = NULL;
	
	command_tree.long_name = "MEASURE";
	command_tree.long_name_length = 7;
	
	command_tree.short_name = "MEAS";
	command_tree.short_name_length = 4;
	
	command_tree.callback = NULL;
	
	measure = &command_tree;
	
	scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 8, "VOLT?", 5, measure_callback);
	scpi_register_command(measure, SCPI_CL_CHILD, "FREQUENCY?", 10, "FREQ?", 5, NULL);
	
	source = scpi_register_command(measure, SCPI_CL_SAMELEVEL, "SOURCE", 6, "SOUR", 4, NULL);
	scpi_register_command(source, SCPI_CL_CHILD, "VOLTAGE", 7, "VOLT", 4, set_voltage);
	
	output = scpi_register_command(measure, SCPI_CL_SAMELEVEL, "OUTPUT", 6, "OUTP", 4, set_output);
	scpi_register_command(measure, SCPI_CL_SAMELEVEL, "OUTPUT?", 7, "OUTP?", 5, get_output);
	scpi_register_command(output, SCPI_CL_CHILD, "STATE", 5, "STAT", 4, set_output);
	scpi_register_command(output, SCPI_CL_CHILD, "STATE?", 6, "STAT?", 5, get_output);
	
	print_command_tree(&command_tree, 0);
	
	putchar('\n');
	
	/*
	printf("\n\nExecuting MEASURE:VOLTAGE?\n");
	*/
	execute_command(&command_tree, "MEASURE:VOLTAGE?");
	execute_command(&command_tree, "SOURCE:VOLTAGE -16.5e-3");
	execute_command(&command_tree, "MEASURE:VOLTAGE?");
	execute_command(&command_tree, "OUTPUT ON");
	execute_command(&command_tree, "MEASURE:VOLTAGE?");
	execute_command(&command_tree, "OUTPUT:STATE?");
	execute_command(&command_tree, "OUTPUT:STATE OFF");
	execute_command(&command_tree, "OUTPUT?");
	
	return 0;
}