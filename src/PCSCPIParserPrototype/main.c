#include <string.h>
#include <stdio.h>

#include "scpiparser.h"

float voltage;
int   voltage_on;

scpi_error_t identify(struct scpi_parser_context* ctx, struct scpi_token* command)
{
	printf("OIC,0.1,SCPI Test,0\n");
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

scpi_error_t measure_callback(struct scpi_parser_context* ctx, struct scpi_token* command)
{
	printf("%e\n", voltage_on ? voltage : 0.0f);
	scpi_free_tokens(command);
	
	return SCPI_SUCCESS;
}

scpi_error_t set_voltage(struct scpi_parser_context* ctx, struct scpi_token* command)
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
		voltage = scpi_parse_numeric(args->value, args->length, 0.0f, 0.0f,1.0e5f).value;
	}
	
	scpi_free_tokens(command);
	
	return SCPI_SUCCESS;
}

scpi_error_t set_output(struct scpi_parser_context* ctx, struct scpi_token* command)
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
			voltage_on = (int)(0.5+scpi_parse_numeric(args->value, args->length, 0.0f, 0.0f, 1.0f).value) ? 1 : 0;
		}
		
	}
	
	scpi_free_tokens(command);
	
	return SCPI_SUCCESS;
}

scpi_error_t get_output(struct scpi_parser_context* ctx, struct scpi_token* command)
{
	printf("%d\n", voltage_on);
	scpi_free_tokens(command);
	return SCPI_SUCCESS;
}

void status_message(char* str)
{
	printf("[ %s ]\n", str);
}

void execute_command(struct scpi_parser_context* ctx, char* str)
{
	scpi_error_t error;
	
	printf(">> %s\n", str);
	error = scpi_execute_command(ctx, str, strlen(str));
	putchar('\n');
	if(error == SCPI_COMMAND_NOT_FOUND)
	{
		struct scpi_error notfound_error;
		notfound_error.id = -100;
		notfound_error.description = "Command error;Command not found";
		notfound_error.length = strlen(notfound_error.description);
		
		scpi_queue_error(ctx, notfound_error);
		
		printf("<< Command not found.\n");
	}
	/*
	printf("<< Error code: %d\n", scpi_execute_command(root, str, strlen(str)));
	*/
}

void print_command_tree(struct scpi_command* list, int tabs)
{
	while(list != NULL)
	{
		int i;
		
		if(list->long_name != NULL)
		{
			for(i = 0; i < tabs; i++)
			{
				putchar('\t');
			}
			
			fwrite(list->long_name, 1, list->long_name_length, stdout);
			putchar('\n');
		}
		print_command_tree(list->children, tabs+1);
		
		list = list->next;
	}
}

void print_single_command(struct scpi_command* list)
{
	if(list->long_name != NULL)
	{
		fwrite(list->long_name, 1, list->long_name_length, stdout);
		putchar('\n');
	}
	print_command_tree(list->children, 1);
}

int main(int argc, char** argv)
{
	struct scpi_parser_context ctx;
	struct scpi_command* measure;
	struct scpi_command* source;
	struct scpi_command* output;
	
	voltage = 0;
	voltage_on = 0;
	
	scpi_init(&ctx);
	measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD,
										"MEASURE", 7, "MEAS", 4, NULL);
	
	printf("\nAssembling command tree:\n\n");
	
	scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);
	
	
	scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 8, "VOLT?", 5, measure_callback);
	scpi_register_command(measure, SCPI_CL_CHILD, "FREQUENCY?", 10, "FREQ?", 5, NULL);
	
	source = scpi_register_command(measure, SCPI_CL_SAMELEVEL, "SOURCE", 6, "SOUR", 4, NULL);
	scpi_register_command(source, SCPI_CL_CHILD, "VOLTAGE", 7, "VOLT", 4, set_voltage);
	
	output = scpi_register_command(measure, SCPI_CL_SAMELEVEL, "OUTPUT", 6, "OUTP", 4, set_output);
	scpi_register_command(measure, SCPI_CL_SAMELEVEL, "OUTPUT?", 7, "OUTP?", 5, get_output);
	scpi_register_command(output, SCPI_CL_CHILD, "STATE", 5, "STAT", 4, set_output);
	scpi_register_command(output, SCPI_CL_CHILD, "STATE?", 6, "STAT?", 5, get_output);
	
	print_command_tree(ctx.command_tree, 0);
	
	putchar('\n');
	
	execute_command(&ctx, "*IDN?");
	execute_command(&ctx, ":MEASURE:VOLTAGE?");
	execute_command(&ctx, ":SOURCE:VOLTAGE 15kV");
	execute_command(&ctx, ":MEASURE:VOLTAGE?");
	execute_command(&ctx, ":OUTPUT:STATE  ON");
	execute_command(&ctx, ":MEASURE:VOLTAGE?");
	execute_command(&ctx, ":OUTPUT:STATE?");
	execute_command(&ctx, ":OUTPUT OFF");
	execute_command(&ctx, ":OUTPUT?");
	execute_command(&ctx, ":SYSTEM:ERROR?");
	execute_command(&ctx, ":CAUSE:AN:ERROR");
	execute_command(&ctx, ":CAUSE:ANOTHER:ERROR");
	execute_command(&ctx, ":SYSTEM:ERROR?");
	execute_command(&ctx, ":SYSTEM:ERROR?");
	execute_command(&ctx, ":SYSTEM:ERROR?");
	
	return 0;
}
