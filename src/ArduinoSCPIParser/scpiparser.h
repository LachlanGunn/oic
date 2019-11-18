/*
Copyright (c) 2013 Lachlan Gunn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef __SCPIPARSER_H
#define __SCPIPARSER_H

#include <Arduino.h>

#ifdef __cplusplus

  extern "C" {
  
#endif

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
struct scpi_error;

typedef scpi_error_t(*command_callback_t)(struct scpi_parser_context*,struct scpi_token*);

struct scpi_token
{
	unsigned char		type;
	
	const char*			value;
	size_t				length;
	
	struct scpi_token*	next;
};

struct scpi_error
{
	int id;
	const char* description;
	size_t length;
	
	struct scpi_error* next;
};

struct scpi_parser_context
{
	struct scpi_command* command_tree;
	struct scpi_error*   error_queue_head;
	struct scpi_error*   error_queue_tail;
};

struct scpi_command
{
	const char*	long_name;
	size_t	long_name_length;
	
	const char*	short_name;
	size_t	short_name_length;

	struct scpi_command* next;
	struct scpi_command* children;
	
	command_callback_t callback;
};

struct scpi_numeric
{
	float  value;
	const char*  unit;
	size_t length;
};

/**
 * Initialise an SCPI parser.
 *
 * @param ctx	A pointer to the struct scpi_parser_context to initialise.
 */
void
scpi_init(struct scpi_parser_context* ctx);

/**
 * Convert an SCPI command into a list of tokens.
 *
 * @param str		A pointer to the string to be parsed.
 * @param length	The length of the string to be parsed.
 *
 * @return A linked list of tokens, pointing into the original string.
 */
struct scpi_token*
scpi_parse_string(const char* str, size_t length);

/**
 * Add a command to a tree.
 *
 * The scpi_register_command function adds a command to the command tree.
 * Each command has a short name, a long name, and optionally a callback.
 *
 * The names are determined by the SCPI standard.  For a single-word name,
 * the long name is equal to the fill name.  For a multi-word name, the
 * long name is equal to the first letter of each word, with the final
 * word spelt out in full.
 *
 * The short name is equal to the first four letters of the long name.
 * If the final letters is a vowel, then it is dropped.
 *
 * For example,
 *
 *		Radial Velocity	-> RVELOCITY	-> RVEL
 *		Sweep			-> SWEEP		-> SWE
 * 
 * @param parent	The command to which the new command is to be attached.
 *
 * @param location	Where to register the command.  If SCPI_CL_SAMELEVEL,
 * 					then the command will be placed at the same level as
 *					the parent, however the value SCPI_CL_CHILD will cause
 *					the new command to be registered at the level beneath
 *					the parent.
 *
 * @param long_name			The long form of the command.
 * @param long_name_length	The length of long_name.
 * @param short_name		The short form of the command.
 * @param short_name_length	The length of short_name.
 *
 * @param callback	A function to be called when the command is executed.
 *
 * @return A pointer to the command structure inserted.
 */
struct scpi_command*
scpi_register_command(struct scpi_command* parent, scpi_command_location_t location,
						const char* long_name,  size_t long_name_length,
						const char* short_name, size_t short_name_length,
						command_callback_t callback);
						
/**
 * Find a command structure in a tree.
 *
 * @param ctx			The parser context as created by scpi_init.
 * @param parsed_string The linked-list of tokens produced by the parser.
 *
 * @return The command object referred to by the token list.
 */
struct scpi_command*
scpi_find_command(struct scpi_parser_context* ctx,
					const struct scpi_token* parsed_string);

					
/**
 * Execute an SCPI command string.
 *
 * @param ctx				The SCPI parser context.
 * @param command_string	The command to be executed.
 * @param length			The length of the executed command.
 *
 * @return An error code.
 */
scpi_error_t
scpi_execute_command(struct scpi_parser_context* ctx, const char* command_string, size_t length);

/**
 * Free a token list.
 *
 * @param start	The token list to be freed.
 */
void
scpi_free_tokens(struct scpi_token* start);

/**
 * Free part of a token list.
 *
 * @param start	The first token to be freed.
 * @param end   The token after the last to be freed.
 */
void
scpi_free_some_tokens(struct scpi_token* start, struct scpi_token* end);

/**
 * Parse a numeric string.
 *
 * The scpi_parse_numeric function parses a decimal numeric string as
 * per the SCPI specification, including units.  When a unit is
 * specified, the SI prefix will be incorporated into the numeric
 * value.  Default, maximum, and minimum values will also be handled.
 *
 * For example, 0.1mV => value: 1e-4, unit: V
 *
 * @param str		The string to parse.
 * @param length	The length of the string to parse.
 * @param min_value     The value of MIN.
 * @param max_value     The value of MAX.
 * @param default_value The value of DEFAULT.
 *
 * @return A structure containing the numeric data.  The unit field
 *			points into the original string.
 */
struct scpi_numeric
scpi_parse_numeric(const char* str, size_t length, float default_value, float min_value, float max_value);

/**
 * Add an error to the queue.
 *
 * @param ctx  	The parser context to which the error is associated.
 * @param error	The error object that is to be queued.
 */
void
scpi_queue_error(struct scpi_parser_context* ctx, struct scpi_error error);

/**
 * Remove the oldest error from the queue.
 *
 * @param ctx	The parser context from which the error is to be popped.
 *
 * @return The oldest error object in the queue.
 */
struct scpi_error*
scpi_pop_error(struct scpi_parser_context* ctx);

#ifdef __cplusplus
  }
#endif

#endif
