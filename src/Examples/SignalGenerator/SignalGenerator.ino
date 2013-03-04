#include <AD9835.h>
#include <scpiparser.h>
#include <Arduino.h>

struct scpi_parser_context ctx;

float frequency;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_frequency(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_frequency(struct scpi_parser_context* context, struct scpi_token* command);

// We begin by creating the AD9835 object with the pin assignments
// that are used.  If another pinout is used, this must be
// modified.
AD9835 dds(
        7, // FSYNC
        3, // SCLK
        2, // SDATA
        6, // FSEL
        5, // PSEL1
        4, // PSEL0
        50000000 // hzMasterClockFrequency (50MHz)
    );

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
   *    :FREQuency  -> set_frequency
   *    :FREQuency? -> get_frequency
   */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);

  source = scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "SOURCE", 6, "SOUR", 4, NULL);
  scpi_register_command(source, SCPI_CL_CHILD, "FREQUENCY", 9, "FREQ", 4, set_frequency);
  scpi_register_command(source, SCPI_CL_CHILD, "FREQUENCY?", 10, "FREQ?", 5, get_frequency);
  
  frequency = 1e3;

  Serial.begin(9600);
  dds.begin();
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;
  
  dds.setFrequencyHz(0, 1000);
  
  dds.selectFrequencyRegister(0);
  
  dds.enable();

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

  Serial.println("OIC,Signal Generator,1,10");
  return SCPI_SUCCESS;
}

/**
 * Read the current frequency.
 */
scpi_error_t get_frequency(struct scpi_parser_context* context, struct scpi_token* command)
{
  float voltage;

  Serial.println(frequency,4);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/**
 * Set the DDS frequency.
 */
scpi_error_t set_frequency(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args;
  struct scpi_numeric output_numeric;
  unsigned char output_value;

  args = command;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  output_numeric = scpi_parse_numeric(args->value, args->length, 1e3, 0, 25e6);
  if(output_numeric.length == 0 ||
    (output_numeric.length == 2 && output_numeric.unit[0] == 'H' && output_numeric.unit[1] == 'z'))
  {
    dds.setFrequencyHz(0, (unsigned long)constrain(output_numeric.value, 0, 25e6));
    frequency = (unsigned long)constrain(output_numeric.value, 0, 25e6);
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

  scpi_free_tokens(command);

  return SCPI_SUCCESS;
}
