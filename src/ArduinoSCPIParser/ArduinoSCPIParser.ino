#include "scpiparser.h"
#include <Arduino.h>

struct scpi_parser_context ctx;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);
  
  Serial.println("OIC,Embedded SCPI Example,1,1");
  return SCPI_SUCCESS;
}

scpi_error_t set_voltage(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;
  
  args = command;
  
  while(args != NULL && args->type == 0)
  {
      args = args->next;
  }
  
  output_numeric = scpi_parse_numeric(args->value, args->length);
  if(output_numeric.length == 0 ||
      (output_numeric.length == 1 && output_numeric.unit[0] == 'V'))
  {
      output_value = (unsigned char)constrain(output_numeric.value / 5.0f * 256.0f, 0, 255);
  }
  else if(output_numeric.length == 2 &&
          output_numeric.unit[0] == 'C' && output_numeric.unit[1] == 'T')
  {
      output_value = (unsigned char)constrain(output_numeric.value, 0, 255);
  }
  else
  {
      scpi_error error;
      error.id = -200;
      error.description = "Command error;Invalid unit";
      error.length = 26;
            
      scpi_queue_error(&ctx, error);
      scpi_free_tokens(command);
      return SCPI_SUCCESS;
  }
  
  analogWrite(3, output_value);
  
  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

void setup()
{
  struct scpi_command* source;

  scpi_init(&ctx);

  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);
  source = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SOURCE", 6, "SOUR", 4, NULL);
  scpi_register_command(source, SCPI_CL_CHILD, "VOLTAGE", 7, "VOLT", 4, set_voltage);

  analogWrite(3, 128);

  Serial.begin(9600);
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;

  while(1)
  {
    read_length = Serial.readBytesUntil('\n', line_buffer, 256);
    if(read_length > 0)
    {
      scpi_execute_command(&ctx, line_buffer, read_length);
    }
  }
}


