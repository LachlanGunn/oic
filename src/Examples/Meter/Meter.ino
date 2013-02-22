#include <scpiparser.h>
#include <Arduino.h>

struct scpi_parser_context ctx;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_voltage(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_voltage_2(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_voltage_3(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_voltage(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_voltage_2(struct scpi_parser_context* context, struct scpi_token* command);

void setup()
{
  struct scpi_command* source;
  struct scpi_command* measure;

  /* First, initialise the parser. */
  scpi_init(&ctx);

  /*
   * After initialising the parser, we set up the command tree.  Ours is
   *
   *  *IDN?         -> identify
   *  :SOURCE
   *    :VOLTage    -> set_voltage
   *    :VOLTage1   -> set_voltage_2
   *  :MEASure
   *    :VOLTage?   -> get_voltage
   *    :VOLTage1?  -> get_voltage_2
   *    :VOLTage2?  -> get_voltage_3
   */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);

  source = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SOURCE", 6, "SOUR", 4, NULL);
  measure = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "MEASURE", 7, "MEAS", 4, NULL);

  scpi_register_command(source, SCPI_CL_CHILD, "VOLTAGE", 7, "VOLT", 4, set_voltage);
  scpi_register_command(source, SCPI_CL_CHILD, "VOLTAGE1", 8, "VOLT1", 5, set_voltage_2);

  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE?", 8, "VOLT?", 5, get_voltage);
  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE1?", 9, "VOLT1?", 6, get_voltage_2);
  scpi_register_command(measure, SCPI_CL_CHILD, "VOLTAGE2?", 9, "VOLT2?", 6, get_voltage_3);

  /*
   * Next, we set our outputs to some default value.
   */
  analogWrite(3, 0);
  analogWrite(5, 0);

  Serial.begin(9600);
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial.readBytesUntil('\n', line_buffer, 256);
    if(read_length > 0)
    {
      scpi_execute_command(&ctx, line_buffer, read_length);
    }
  }
}


/*
 * Respond to *IDN?
 */
scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  Serial.println("OIC,Embedded SCPI Example,1,10");
  return SCPI_SUCCESS;
}

/**
 * Read the voltage on A0.
 */
scpi_error_t get_voltage(struct scpi_parser_context* context, struct scpi_token* command)
{
  float voltage;

  voltage = analogRead(0) * 5.0f/1024;
  Serial.println(voltage,4);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/**
 * Read the voltage on A1.
 */
scpi_error_t get_voltage_2(struct scpi_parser_context* context, struct scpi_token* command)
{
  float voltage;

  voltage = analogRead(1) * 5.0f/1024;
  Serial.println(voltage,4);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/**
 * Read the voltage on A2.
 */
scpi_error_t get_voltage_3(struct scpi_parser_context* context, struct scpi_token* command)
{
  float voltage;

  voltage = analogRead(2) * 5.0f/1024;
  Serial.println(voltage,4);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/**
 * Set the voltage using PWM on pin 3.
 */
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

  output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 5);
  if(output_numeric.length == 0 ||
    (output_numeric.length == 1 && output_numeric.unit[0] == 'V'))
  {
    output_value = (unsigned char)constrain(output_numeric.value / 5.0f * 256.0f, 0, 255);
  }
  else if(output_numeric.length == 2 &&
    output_numeric.unit[0] == 'N' && output_numeric.unit[1] == 'T')
  {
    output_value = (unsigned char)constrain(output_numeric.value, 0, 255);
  }
  else
  {
    scpi_error error;
    error.id = -200;
    error.description = "Command error;Invalid unit";
    error.length = 26;
    Serial.print(output_numeric.length);

    scpi_queue_error(&ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  analogWrite(3, output_value);

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

/**
 * Set the voltage using PWM on pin 5.
 */
scpi_error_t set_voltage_2(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 0, 0, 5);
  if(output_numeric.length == 0 ||
    (output_numeric.length == 1 && output_numeric.unit[0] == 'V'))
  {
    output_value = (unsigned char)constrain(output_numeric.value / 5.0f * 256.0f, 0, 255);
  }
  else if(output_numeric.length == 2 &&
    output_numeric.unit[0] == 'N' && output_numeric.unit[1] == 'T')
  {
    output_value = (unsigned char)constrain(output_numeric.value, 0, 255);
  }
  else
  {
    scpi_error error;
    error.id = -200;
    error.description = "Command error;Invalid unit";
    error.length = 26;
    Serial.print(output_numeric.length);

    scpi_queue_error(&ctx, error);
    scpi_free_tokens(command);
    return SCPI_SUCCESS;
  }

  analogWrite(5, output_value);

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}

