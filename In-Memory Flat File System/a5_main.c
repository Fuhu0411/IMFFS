#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include "a5_imffs.h"

#define DEFAULT_BLOCK_COUNT 64
#define MAX_COMMAND 1024
#define WHITESPACE " \t"

#define HANDLE_RESULT(e) handle_result(e, #e)

int handle_result(IMFFSResult result, char *action) {
  int modified_result = 0;
  
  switch (result) {
    case IMFFS_OK:
      // fprintf(stderr, "Successfully executed '%s'\n", action);
      break;
    case IMFFS_ERROR:
      fprintf(stderr, "Error when executing '%s'\n", action);
      break;
    case IMFFS_FATAL:
      fprintf(stderr, "Fatal error when executing '%s'\n", action);
      modified_result = 1;
      break;
    case IMFFS_INVALID:
      fprintf(stderr, "Invalid result when executing '%s'\n", action);
      modified_result = 1; 
      break;
    case IMFFS_NOT_IMPLEMENTED:
      fprintf(stderr, "Function not implemented when executing '%s'\n", action);
      break;
    default:
      // this shouldn't happen
      fprintf(stderr, "Unknown return value when executing '%s'\n", action);
      assert(0);
      modified_result = -1;
      break;
  }
  
  return modified_result;
}

int interactive_imffs(uint32_t block_count) {
  int result = 0, len, help;
  IMFFSPtr fs = NULL;
  char command[MAX_COMMAND], ch, *token, *token2;
  
  while (!result) {
    if (NULL == fs) {
      // printf("Creating a file system with %u blocks.\n", block_count);
      result = HANDLE_RESULT(imffs_create(block_count, &fs));
      if (NULL == fs) {
        result = -1;
      }
    } else {

      printf("> ");
      if (NULL == fgets(command, MAX_COMMAND, stdin)) {
        printf("\n\nQuitting on EOF.\n");
        result = 1;
      } else {
        len = strlen(command);
        ch = command[len - 1];

        if (ch != '\n') {
          printf("Command exceeds %d characters, unable to process.\n", MAX_COMMAND-1);
          while (ch != '\n') {
            ch = fgetc(stdin);
          }
        } else {
          command[len - 1] = '\0';
          
          // printf("You entered: '%s'\n", command);
          token = strtok(command, WHITESPACE);
          
          help = 0;
          if (NULL == token) {
            help = 1;
          } else if (0 == strcasecmp("save", token)) {
            token = strtok(NULL, WHITESPACE);
            token2 = strtok(NULL, WHITESPACE);
            if (NULL == token || NULL == token2 || NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_save(fs, token, token2));
            }
          } else if (0 == strcasecmp("load", token)) {
            token = strtok(NULL, WHITESPACE);
            token2 = strtok(NULL, WHITESPACE);
            if (NULL == token || NULL == token2 || NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_load(fs, token, token2));
            }
          } else if (0 == strcasecmp("delete", token)) {
            token = strtok(NULL, WHITESPACE);
            if (NULL == token || NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_delete(fs, token));
            }
          } else if (0 == strcasecmp("rename", token)) {
            token = strtok(NULL, WHITESPACE);
            token2 = strtok(NULL, WHITESPACE);
            if (NULL == token || NULL == token2 || NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_rename(fs, token, token2));
            }
          } else if (0 == strcasecmp("dir", token)) {
            if (NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_dir(fs));
            }
          } else if (0 == strcasecmp("fulldir", token)) {
            if (NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_fulldir(fs));
            }
          } else if (0 == strcasecmp("defrag", token)) {
            if (NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              result = HANDLE_RESULT(imffs_defrag(fs));
            }
          } else if (0 == strcasecmp("help", token)) {
            help = 1;
          } else if (0 == strcasecmp("quit", token)) {
            if (NULL != strtok(NULL, "")) {
              help = 1;
            } else {
              printf("\nQuitting.\n");
              result = 1;
            }
          } else {
            printf("Unknown command '%s'\n", token);
            help = 1;
          }
          
          if (help) {
            printf("\nCommands:\n\n");
            printf("save diskfile imffsfile: copy from your system to IMFFS\n");
            printf("load imffsfile diskfile: copy from IMFFS to your system\n");
            printf("delete imffsfile: remove the IMFFS file from the system, allowing the blocks to be used for other files\n");
            printf("rename imffsold imffsnew: rename the IMFFS file from imffsold to imffsnew, keeping all of the data intact\n");
            printf("dir: will list all of the files and the number of bytes they occupy\n");
            printf("fulldir: is like \"dir\" except it shows a the files and details about all of the chunks they are stored in (where, and how big)\n");
            printf("defrag: is described below\n");
            printf("help: lists the commands\n");
            printf("quit: will quit the program\n\n");
          }
        }
      }
    }
  }
  
  if (result >= 0) {
    result = HANDLE_RESULT(imffs_destroy(fs));
    if (result > 0) {
      result = 0;
    }
  }
  
  return result;
}

int main(int argc, char *argv[]) {
  int result = 0;
  int opt;

  uint32_t block_count = DEFAULT_BLOCK_COUNT;
  long converted;
  char *end_p;

  while ((0 == result) && (opt = getopt(argc, argv, "b:h")) != -1) {
    switch (opt) {
    case 'b':
      converted = strtol(optarg, &end_p, 10);
      if (end_p == optarg || converted < 1 || converted > UINT32_MAX) {
        fprintf(stderr, "Number of blocks must be between 1 and %u\n", UINT32_MAX);
        block_count = DEFAULT_BLOCK_COUNT;
      } else {
        block_count = (uint32_t)converted;
      }
      break;
    case 'h':
      result = -1;
      break;
    default:
      result = -1;
      break;
    }
  }
  
  if (result < 0 || argc > optind) {
    fprintf(stderr, "Usage: %s [-b block_count]\n", argv[0]);
  } else {
    result = interactive_imffs(block_count);
  }
  
  return result;
}