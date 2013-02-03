#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "scpiparser.h"

struct scpi_token*
scpi_parse_string(char* str, size_t length)
{
	int i;
	
	struct scpi_token* head;
	struct scpi_token* tail;
	
	int token_start;
	
	head = NULL;
	tail = NULL;
	token_start = 0;
	
	for(i = 0; i < length; i++)
	{
		
		if(str[i] == ':' || str[i] == ' ' || i == length-1)
		{
			struct scpi_token* new_tail;
			
			new_tail = (struct scpi_token*)malloc(sizeof(struct scpi_token));
			new_tail->type = 0;
			new_tail->value = str+token_start;
			new_tail->length = i-token_start;
			new_tail->next = NULL;
			
			if(i == length-1)
			{
				new_tail->length++;
			}
						
			if(tail == NULL)
			{
				head = new_tail;
			}
			else
			{
				tail->next = new_tail;
			}
			tail = new_tail;
			
			token_start = i+1;
			
			if(str[i] == ' ')
			{
				break;
			}
		}
	}
	
	for(i++; i < length; i++)
	{
		if(str[i] == ',' || i == length-1)
		{
			struct scpi_token* new_tail;
			new_tail = (struct scpi_token*)malloc(sizeof(*new_tail));
			new_tail->type = 1;
			new_tail->value = str+token_start;
			new_tail->length = i-token_start;
			new_tail->next = NULL;
			
			if(i == length-1)
			{
				new_tail->length++;
			}
			
			tail->next = new_tail;
			tail = new_tail;

			token_start = i+1;
		}
	}
	
	return head;
}

struct scpi_command*
scpi_register_command(struct scpi_command* parent, scpi_command_location_t location,
						char* long_name,  size_t long_name_length,
						char* short_name, size_t short_name_length,
						command_callback_t callback)
{
	
	struct scpi_command* current_command;
	
	if(location == SCPI_CL_CHILD)
	{
		current_command = parent->children;
	}
	else
	{
		current_command = parent;
	}
	
	if(current_command == NULL)
	{
		parent->children = (struct scpi_command*)malloc(sizeof(struct scpi_command));
		current_command = parent->children;
	}
	else
	{
		while(current_command->next != NULL)
		{
			current_command = current_command->next;
		}
		
		current_command->next = (struct scpi_command*)malloc(sizeof(struct scpi_command));
		current_command = current_command->next;
	}
	
	current_command->next = NULL;
	current_command->children = NULL;
	
	current_command->long_name = long_name;
	current_command->long_name_length = long_name_length;
	
	current_command->short_name = short_name;
	current_command->short_name_length = short_name_length;
	
	current_command->callback = callback;
	
	return current_command;
}

struct scpi_command*
scpi_find_command(struct scpi_command* root,
					struct scpi_token* parsed_string)
{
	struct scpi_token* current_token;
	struct scpi_command* current_command;
	
	current_token = parsed_string;
	current_command = root;

	
	while(current_token != NULL && current_token->type == 0)
	{
	
		int found_token = 0;
		while(current_command != NULL)
		{
			
			if((current_token->length == current_command->long_name_length
					&& !memcmp(current_token->value, current_command->long_name, current_token->length))
				|| (current_token->length == current_command->short_name_length
					&& !memcmp(current_token->value, current_command->short_name, current_token->length)))
			{
				/* We have found the token. */
				current_token = current_token->next;
				
				if(current_token == NULL || current_token->type != 0)
				{
					return current_command;
				}
				else
				{
					found_token = 1;
					current_command = current_command->children;
					break;
				}
			}
			else
			{
				current_command = current_command->next;
			}
		}
		
		if(!found_token)
		{
			return NULL;
		}
	}
	
	return NULL;
}

scpi_error_t
scpi_execute_command(struct scpi_command* root, char* command_string, size_t length)
{
	struct scpi_command* command;
	struct scpi_token* parsed_command;
	
	parsed_command = scpi_parse_string(command_string, length);
	
	command = scpi_find_command(root, parsed_command);
	if(command == NULL)
	{
		return SCPI_COMMAND_NOT_FOUND;
	}
	
	if(command->callback == NULL)
	{
		return SCPI_NO_CALLBACK;
	}
	
	
	return command->callback(parsed_command);
}

void
scpi_free_some_tokens(struct scpi_token* start, struct scpi_token* end)
{
	struct scpi_token* prev;
	while(start != NULL && start != end)
	{
		prev = start;
		start = start->next;
		
		free((void*)prev);
	}
}

void
scpi_free_tokens(struct scpi_token* start)
{
	scpi_free_some_tokens(start, NULL);
}

float
scpi_parse_numeric(char* str, size_t length)
{
	int i;
	long mantissa;
	int state;
	int sign;
	int point_position;
	int exponent;
	int exponent_sign;
	long exponent_multiplier;
	float value;
	
	exponent = 0;
	exponent_sign = 0;
	point_position = 0;
	sign = 0;
	state = 0;
	mantissa = 0;
	exponent_multiplier = 1;

	for(i = 0; i < length; i++)
	{
		if(state == 0)
		{
			/* Remove leading whitespace */
			
			if(isspace(str[i]))
			{
				continue;
			}
			else if(str[i] == '+' || str[i] == '-')
			{
				/* We have hit a +/- */
				state = 1;
			}
			else if(isdigit(str[i]))
			{
				/* We have reached the number itself. */
				state = 2;
			}
			else
			{
				state = -1;
				continue;
			}
		}
		
		if(state == 1)
		{
			/* Set the sign. */
			if(str[i] == '+')
			{
				sign = 0;
			}
			else
			{
				sign = 1;
			}
			
			state = 2;
			continue;
		}
		
		if(state == 2 || state == 3)
		{
			if(isdigit(str[i]))
			{
				/* Start accumulating digits. */
				mantissa = (10*mantissa) + (long)(str[i] - 0x30);
				
				if(state == 3)
				{
					/* We are past the decimal point, so reposition it. */
					point_position++;
				}
			}
			else if(str[i] == '.')
			{
				state = 3;
				continue;
			}
			else if(str[i] == 'e')
			{
				state = 4;
				continue;
			}
			else
			{
				state = -1;
			}
		}
		
		if(state == 4)
		{
			/* We are now looking at the exponent sign. */
			if(str[i] == '+' || str[i] == '-')
			{
				if(str[i] == '-')
				{
					exponent_sign = 1;
				}
			}
			else if(isdigit(str[i]))
			{
				state = 5;
			}
			else
			{
				state = -1;
			}
		}
		
		if(state == 5)
		{
			
			if(isdigit(str[i]))
			{
				exponent = (exponent*10) + (int)(str[i] - 0x30);
				continue;
			}
			else
			{
				state = -1;
			}
		}
	}
	
	value = (float)mantissa;
	
	if(exponent_sign != 0)
	{
		exponent = -exponent;
	}
	
	exponent -= point_position;
	if(exponent > 0)
	{
		for(i = 0; i < exponent; i++)
		{
			exponent_multiplier *= 10;
		}
		
		value *= exponent_multiplier;
	}
	else
	{
		for(i = exponent; i < 0; i++)
		{
			exponent_multiplier *= 10;
		}
		
		value /= exponent_multiplier;
	}
	
	if(sign != 0)
	{
		value = -value;
	}
	
	
	return value;
}