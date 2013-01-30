#ifndef __SCPIPARSER_H
#define __SCPIPARSER_H

typedef enum scpi_error_codes
{
	SCPI_SUCCESS			=  0,
	SCPI_COMMAND_NOT_FOUND	= -1,
	SCPI_NO_CALLBACK		= -2
} scpi_error_t;

struct scpi_token;
struct scpi_parser_context;
struct scpi_command;

typedef scpi_error_t(*command_callback_t)(struct scpi_token*);

struct scpi_token
{
	unsigned char		type;
	
	char*				value;
	size_t				length;
	
	struct scpi_token*	next;
};

struct scpi_parser_context
{
	struct scpi_token* head;
};

struct scpi_command
{
	char*	long_name;
	size_t	long_name_length;
	
	char*	short_name;
	size_t	short_name_length;

	struct scpi_command* next;
	struct scpi_command* children;
	
	command_callback_t callback;
};


struct scpi_token*
scpi_parse_string(char* str, size_t length);

struct scpi_command*
scpi_register_command(struct scpi_command* parent,
						char* long_name,  size_t long_name_length,
						char* short_name, size_t short_name_length,
						command_callback_t callback);
						
struct scpi_command*
scpi_find_command(struct scpi_command* root,
					struct scpi_token* parsed_string);
					
scpi_error_t
scpi_execute_command(struct scpi_command* root, char* command_string, size_t length);

#endif