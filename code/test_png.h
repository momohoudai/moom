/* date = January 27th 2022 9:19 am */

#ifndef TEST_PNG_H
#define TEST_PNG_H



void test_png() {
  struct {
    const char* in;
    const char* out;
  } test_cases[] = 
  {
    test_assets_dir("test_in0.png"), "out0.png",
    test_assets_dir("test_in1.png"), "out1.png",
    test_assets_dir("test_in2.png"), "out2.png",
    test_assets_dir("test_in3.png"), "out3.png",
    test_assets_dir("test_in4.png"), "out4.png",
    test_assets_dir("test_in5.png"), "out5.png",
  }; 
  
  U32 memory_size = MB(10);
  U8* memory = (U8*)malloc(memory_size);
  if (!memory) { 
    test_log("Cannot allocate memory\n");
    return;
  }
  
  for (int i = 0; i < ArrayCount(test_cases); ++i)
  {
    test_log("Test Case: %d\n", i);
    test_create_log_section_until_scope;
    
    Arena app_arena = create_arena(memory, memory_size);
    Memory png_file = test_read_file_to_memory(&app_arena, test_cases[i].in);
    
    if (!is_ok(png_file)){
      test_log("Cannot read file: %d\n", i);
      continue;
    }
    
    Image bitmap = read_png(png_file, &app_arena);
    if (!is_ok(bitmap)) {
      test_log("Read PNG failed: %d\n", i);
      continue;
    }
    test_log("Read success: %d\n", i);
    
    Memory png_output = write_png(bitmap,
                                  &app_arena); 
    if (!is_ok(png_output)) {
      test_log("Write to memory failed: %d\n", i);
      continue;
    }
    
    if(!test_write_memory_to_file(png_output, test_cases[i].out)) {
      test_log("Cannot write to file: %d\n", i);
      continue;
    }
    test_log("Wrote to file\n");
  }
  
  free(memory);
  
}

#endif //TEST_PNG_H