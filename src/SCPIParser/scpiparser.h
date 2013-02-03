#ifndef __SCPIPARSER_H
#define __SCPIPARSER_H

typedef enum scpi_error_codes
{
	SCPI_SUCCESS			=  0,
	SCPI_COMMAND_NOT_FOUND	= -1,
	SCPI_NO_CALLBACK		= -2
} scpi_error_t;

typedef enum scpi_command_location
{
	SCPI_CL_SAMELEVEL,
	SCPI_CL_CHILD
} scpi_command_location_t;

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
scpi_register_command(struct scpi_command* parent, scpi_command_location_t location,
						char* long_name,  size_t long_name_length,
						char* short_name, size_t short_name_length,
						command_callback_t callback);
						
struct scpi_command*
scpi_find_command(struct scpi_command* root,
					struct scpi_token* parsed_string);
					
scpi_error_t
scpi_execute_command(struct scpi_command* root, char* command_string, size_t length);

void
scpi_free_tokens(struct scpi_token* start);

void
scpi_free_some_tokens(struct scpi_token* start, struct scpi_token* end);

float
scpi_parse_numeric(char* str, size_t length);

#endif