// Authors: Gerald Wong, momodevelop
//
// This file processes PNG files
// - Extracts PNG information
// - Reads PNG memoryinto 32-bit bitmap format
// - Writes 32-bit bitmap format to PNG memory
//
// NOTES:
//   - Only supports little-endian OS 
//   - Only reads and writes 32-bit RGBA formats.
//
// TODO:
//   - Support other formats for reading/writing
//
#ifndef MOMO_PNG
#define MOMO_PNG

typedef struct {
  U8* data;
  UMI data_size;
  
  U32 width;
  U32 height;
  U8 bit_depth;
  U8 colour_type;
  U8 compression_method;
  U8 filter_method;
  U8 interlace_method;
} PNG;

static B32     png_read(PNG* png, void* png_memory, UMI png_size);

// TODO: remove
static B32     png_read_from_blk(PNG* p, Block blk);


static Image32 png_to_img32(PNG* png, Arena* allocator);
//static U32*    png_rasterize( 

static Block   png_write_img32_to_blk(Image32 img, Arena* allocator);


//static void*   png_write(UMI* out_size);

///////////////////////////////////////////////////////////////
// IMPLEMENTATION
// We are only interested in 4-channel images in RGBA format
#define _PNG_CHANNELS 4 


typedef struct {
  Stream stream;
  Arena* allocator; 
  
  Stream image_stream;
  U32 image_width;
  U32 image_height;
  
  
  Stream unfiltered_image_stream; // for filtering and deflating
  
  // other useful info
  U32 bit_depth;
  
  Stream compressed_image_stream;
}_PNG_Context;


typedef struct {
  U8 signature[8];
}_PNG_Header; 

// 5.3 Chunk layout
// | length | type | data | CRC
typedef struct {
  U32 length;
  union {
    U32 type_U32;
    U8 type[4];
  };
}_PNG_Chunk_Header;


#pragma pack(push, 1)
typedef struct {
  U32 width;
  U32 height;
  U8 bit_depth;
  U8 colour_type;
  U8 compression_method;
  U8 filter_method;
  U8 interlace_method;
}_PNG_IHDR;
#pragma pack(pop)

typedef struct  {
  U32 crc; 
}_PNG_Chunk_Footer;

// ZLIB header notes:
// Bytes[0]:
// - compression flags bit 0-3: Compression Method (CM)
// - compression flags bit 4-7: Compression Info (CINFO)
// Bytes[1]:
// - additional flags bit 0-4: FCHECK 
// - additional flags bit 5: Preset dictionary (FDICT)
// - additional flags bit 6-7: Compression level (FLEVEL)
typedef struct {
  U8 compression_flags;
  U8 additional_flags;
}_PNG_IDAT_Header;


typedef struct  {
  // Canonical ordered symbols
  U16* symbols; 
  U32 symbol_count;
  
  // Number of symbols per length
  // i.e. code_lengths[1] is the number of symbols with length 1.
  U16* lengths;
  U32 length_count;
}_PNG_Huffman;

// Modified from Annex D of PNG specification:
// https://www.w3.org/TR/2003/REC-PNG-20031110/#D-CRCAppendix
// crc variable indicates the starting register
static U32
_png_calculate_crc32(U8* data, U32 data_size) {
  static const U32 crc_table[256] =
  {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
  };
  
  U32 r = 0xFFFFFFFFL;
  for (U32 i = 0; i < data_size; ++i) {
    r = crc_table[(r ^ data[i]) & 0xFF] ^ (r >> 8);
  }
  return r ^ 0xFFFFFFFFL;
}




static S32
_png_huffman_decode(Stream* src_stream, _PNG_Huffman huffman) {
  S32 code = 0;
  S32 first = 0;
  S32 index = 0;
  
  for (U32 len = 1; 
       len <= huffman.length_count - 1; 
       ++len) 
  {
    U32 bits = srm_consume_bits(src_stream, 1);
    code |= bits;
    S32 count = huffman.lengths[len];
    if(code - count < first) {
      return huffman.symbols[index + (code - first)];
    }
    index += count;
    first += count;
    first <<= 1;
    code <<= 1;
  }
  
  return -1;
}

// NOTE(Momo): 
// https://datatracker.ietf.org/doc/html/rfc1951
// Section 3.2.2
static void
_png_huffman_compute(_PNG_Huffman* h,
                     Arena* allocator, 
                     U16* codes,
                     U32 codes_size, 
                     U32 max_lengths) 
{
  _PNG_Huffman ret = {};
  
  // Each code corresponds to a symbol
  h->symbol_count = codes_size;
  h->symbols = arn_push_arr(U16, allocator, codes_size);
  zero_memory(h->symbols, h->symbol_count * sizeof(U16));
  
  
  // We add +1 because lengths[0] is not possible
  h->length_count = max_lengths + 1;
  h->lengths = arn_push_arr(U16, allocator, max_lengths + 1);
  zero_memory(h->lengths, h->length_count * sizeof(U16));
  
  // 1. Count the number of codes for each code length
  for (U32 sym = 0; sym < codes_size; ++sym)  {
    U16 len = codes[sym];
    ++h->lengths[len];
  }
  
  // 2. Numerical value of smallest code for each code length
  Arena_Marker mark = arn_mark(allocator);
  
  U16* len_offset_table = arn_push_arr(U16, allocator, max_lengths+1);
  zero_memory(len_offset_table, (max_lengths+1) * sizeof(U16));
  
  for (U32 len = 1; len < max_lengths; ++len) {
    len_offset_table[len+1] = len_offset_table[len] + h->lengths[len]; 
  }
  
  // 3. Assign numerical values to all codes
  for (U32 sym = 0; sym < codes_size; ++sym)
  {
    U16 len = codes[sym];
    if (len > 0) {
      U16 code = len_offset_table[len]++;
      h->symbols[code] = (U16)sym;
    }
  }
  arn_revert(mark); 
  
}


static B32
_png_deflate(Stream* src_stream, Stream* dest_stream, Arena* allocator) 
{
  
  static const U16 lens[29] = { /* Size base for length codes 257..285 */
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
  static const U16 len_ex_bits[29] = { /* Extra bits for length codes 257..285 */
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
  static const U16 dists[30] = { /* Offset base for distance codes 0..29 */
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577};
  static const U16 dist_ex_bits[30] = { /* Extra bits for distance codes 0..29 */
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
    12, 12, 13, 13 };
  
  
  U8 BFINAL = 0;
  while(BFINAL == 0){
    arn_set_revert_point(allocator);
    
    BFINAL = (U8)srm_consume_bits(src_stream, 1);
    U16 BTYPE = (U8)srm_consume_bits(src_stream, 2);
    switch(BTYPE) {
      case 0b00: {
        srm_flush_bits(src_stream);
        
        srm_consume_bits(src_stream, 5);
        U16 LEN = (U16)srm_consume_bits(src_stream, 16);
        U16 NLEN = (U16)srm_consume_bits(src_stream, 16);
        if ((U16)LEN != ~((U16)(NLEN))) {
          return false; 
        }
        // TODO(Momo): support this type?
        return false;
      } break;
      case 0b01: 
      case 0b10: {
        _PNG_Huffman lit_huffman = {};
        _PNG_Huffman dist_huffman = {};
        
        if (BTYPE == 0b01) {
          // Fixed huffman
          U16 lit_codes[288] = {};
          U16 dist_codes[32] = {};
          
          U32 lit = 0;
          for (; lit < 144; ++lit) {
            lit_codes[lit] = 8;
          }
          for (; lit < 256; ++lit) {
            lit_codes[lit] = 9;
          }
          for (; lit < 280; ++lit) {
            lit_codes[lit] = 7;
          }
          for (; lit < array_count(lit_codes); ++lit) {
            lit_codes[lit] = 8;
          }
          for (lit = 0; lit < array_count(dist_codes); ++lit) {
            dist_codes[lit] = 5;
          }
          
          
          _png_huffman_compute(&lit_huffman,
                               allocator, 
                               lit_codes, 
                               array_count(lit_codes),
                               15);
          _png_huffman_compute(&dist_huffman,
                               allocator,
                               dist_codes,
                               array_count(dist_codes),
                               15);
          
        }
        else // BTYPE == 0b10
        {
          U32 HLIT = srm_consume_bits(src_stream, 5) + 257;
          U32 HDIST = srm_consume_bits(src_stream, 5) + 1;
          U32 HCLEN = srm_consume_bits(src_stream, 4) + 4;
          
          static const U32 order[] = {
            16, 17, 18, 0, 8 ,7, 9, 6, 10, 5, 
            11, 4, 12, 3, 13, 2, 14, 1, 15,
          };
          
          U16 code_codes[19] = {};
          
          for(U32 i = 0; i < HCLEN; ++i) {
            code_codes[order[i]] = (U16)srm_consume_bits(src_stream, 3);
          }
          
          _PNG_Huffman code_huffman = {};
          _png_huffman_compute(&code_huffman,
                               allocator,
                               code_codes,
                               array_count(code_codes),
                               15); 
          
         
          U16* lit_dist_codes = arn_push_arr(U16, allocator, HDIST + HLIT);
          
          // NOTE(Momo): Decode
          // Loop until end of block code recognize
          U32 last_len = 0;
          for(U32 i = 0; i < (HDIST + HLIT);) {
            
            S32 sym = _png_huffman_decode(src_stream, code_huffman);
            
            if(sym >= 0 && sym <= 15) {
              lit_dist_codes[i++] = (U16)sym;
            }
            else 
            {	
              U32 times_to_repeat = 0;
              U16 code_to_repeat = 0;
              if (sym == 16) {
                // Copy the previous code length 3-6 times
                if (i == 0) return false;
                
                times_to_repeat = 3 + srm_consume_bits(src_stream, 2);
                code_to_repeat = lit_dist_codes[i-1];
                
              }
              
              else if (sym == 17) {
                // Repeat a code length of 0 for 3-10 times
                times_to_repeat = 3 + srm_consume_bits(src_stream, 3);
              }
              else if (sym == 18) {
                // Repeat a code length of 0 for 11-138 times
                times_to_repeat = 11 + srm_consume_bits(src_stream, 7);
              }
              else {
                // Invalid symbol
                return false;
              }
              
              while(times_to_repeat--) {
                lit_dist_codes[i++] = code_to_repeat;
              }
              
            }
            
          }
          
          _png_huffman_compute(&lit_huffman,
                               allocator, 
                               lit_dist_codes, 
                               HLIT,
                               15);
          _png_huffman_compute(&dist_huffman,
                               allocator,
                               lit_dist_codes + HLIT,
                               HDIST,
                               15);					
        }
        
        static int pass =0;
        ++pass;
        int wtf = 0;
        
        // NOTE(Momo): Actual decoding
        for (;;) 
        {
          ++wtf;
          
          S32 sym = _png_huffman_decode(src_stream, lit_huffman);
          if (pass == 2) {
            //test_log("%d\n", sym);
          }
          //_png_log("sym: %d\n", sym);
          
          // NOTE(Momo): Normal case
          if (sym <= 255) { 
            U8 byte_to_write = (U8)(sym & 0xFF); 
            srm_write(dest_stream, byte_to_write);
          }
          // NOTE(Momo): Extra code case
          else if (sym >= 257) {
            
            sym -= 257;
            if (sym >= 29) {
              return false;
            }
            U32 len = lens[sym];
            if (len_ex_bits[sym]) len += srm_consume_bits(src_stream, len_ex_bits[sym]);
            
            sym = _png_huffman_decode(src_stream, dist_huffman);
            if (sym < 0) return false;
            
            U32 dist = dists[sym];
            if (dist_ex_bits[sym]) dist += srm_consume_bits(src_stream, dist_ex_bits[sym]);
            
            
            // test_log("%d\n", len);
            while(len--) {
              UMI target_index = dest_stream->pos - dist;
              U8 byte_to_write = dest_stream->data[target_index];
              srm_write(dest_stream, byte_to_write);
            }
          }
          else { 
            // sym == 256
            break;
          }
        }
      } break;
      default: {
        return false;
      }
    }
  }
  return true;
}




static U32 
_png_get_channels_from_colour_type(U32 colour_type) {
  // NOTE(Momo): Determine the channels
  // colour_type 1 = Pallete used
  // colour_type 2 = Colour used 
  // colour_type 4 = alpha used
  switch(colour_type){
    case 0: {
      return 1; // Grayscale
    } break;
    case 2: {
      return 3; // RGB
    } break;
    case 3: { // Palette
      return 0;
    } break;
    case 4: {
      return 2; // Grayscale + alpha
    } break;
    case 6: { 
      return 4; // RGBA
    } break;
    default: {
      return 0;
    }
  }
}

static B32
_png_is_format_supported(_PNG_IHDR* IHDR){
  if (IHDR->colour_type != 6 ||
      IHDR->bit_depth != 8 ||
      IHDR->compression_method != 0 ||
      IHDR->filter_method != 0 ||
      IHDR->interlace_method != 0) 
  {
    
    return false;
  }
  return true;
}

static B32
_png_is_signature_valid(U8* comparee) {
  static const U8 signature[] = { 
    137, 80, 78, 71, 13, 10, 26, 10 
  };
  
  for (U32 i = 0; i < array_count(signature); ++i) {
    if (signature[i] != comparee[i]) {
      return false;
    }
  }
  
  return true;
}

//~ NOTE(Momo): Filtering
static B32
_png_filter_none(_PNG_Context* c) {
  U32 bpl = c->image_width * _PNG_CHANNELS; // bytes per line
  for (U32 i = 0; i < bpl; ++i ){
    U8* pixel_byte = srm_consume(U8, &c->unfiltered_image_stream);
    if (pixel_byte == null) {
      return false;
    }
    srm_write(&c->image_stream, *pixel_byte);
  }
  return true;
}

static B32
_png_filter_sub(_PNG_Context* c) {
  U32 bpp = _PNG_CHANNELS; // bytes per pixel
  U32 bpl = c->image_width * _PNG_CHANNELS; // bytes per line
  for (U32 i = 0; i < bpl; ++i ){
    
    U8* pixel_byte_p = srm_consume(U8, &c->unfiltered_image_stream);
    if (pixel_byte_p == null)return false;
    
    U8 pixel_byte = (*pixel_byte_p); // sub(x)
    if (i < bpp) {
      srm_write(&c->image_stream, pixel_byte);
    }
    else {
      UMI current_index = c->image_stream.pos;
      U8 left_reference = c->image_stream.data[current_index - bpp]; // Raw(x-bpp)
      U8 pixel_byte_to_write = (pixel_byte + left_reference) % 256;  
      
      srm_write(&c->image_stream, pixel_byte_to_write);
    }
    
  }    
  
  return true;
}

static B32
_png_filter_average(_PNG_Context* c) {
  U32 bpp = _PNG_CHANNELS; // bytes per pixel
  U32 bpl = c->image_width * _PNG_CHANNELS; // bytes per line
  
  for (U32 i = 0; i < bpl; ++i ){
    
    U8* pixel_byte_p = srm_consume(U8, &c->unfiltered_image_stream);
    if (pixel_byte_p == null) return false;
    
    U8 pixel_byte = (*pixel_byte_p); // sub(x)
    
    UMI current_index = c->image_stream.pos;
    U8 left = (i < bpp) ? 0 :  c->image_stream.data[current_index - bpp]; // Raw(x-bpp)
    U8 top = (current_index < bpl) ? 0 : c->image_stream.data[current_index - bpl]; // Prior(x)
    
    // NOTE(Momo): Formula uses floor((left+top)/2). 
    // Integer Truncation should do the job!
    U8 pixel_byte_to_write = (pixel_byte + (left + top)/2) % 256;  
    
    srm_write(&c->image_stream, pixel_byte_to_write);
  }
  
  
  return true;
}

static B32
_png_filter_paeth(_PNG_Context* cx) {
  U32 bpp = _PNG_CHANNELS; // bytes per pixel
  U32 bpl = cx->image_width * _PNG_CHANNELS; // bytes per line
  
  for (U32 i = 0; i < bpl; ++i ){
    U8* pixel_byte_p = srm_consume(U8, &cx->unfiltered_image_stream);
    if (pixel_byte_p == null) return false;
    U8 pixel_byte = (*pixel_byte_p); // Paeth(x)
    
    // NOTE(Momo): PaethPredictor
    // https://www.w3.org/TR/PNG-Filters.html
    U8 paeth_predictor; 
    {
      UMI current_index = cx->image_stream.pos;
      
      // respectively: left, top, top left
      S32 a, b, c;
      
      a = (i < bpp) ? 0 : (S32)(cx->image_stream.data[current_index - bpp]); // Raw(x-bpp)
      b = (current_index < bpl) ? 0 : (S32)(cx->image_stream.data[current_index - bpl]); // Prior(x)
      c = (i < bpp || current_index < bpl) ? 0 : (S32)(cx->image_stream.data[current_index - bpl - bpp]); // Prior(x)
      
      S32 p = a + b - c; //initial estimate
      S32 pa = abs_of(p - a);
      S32 pb = abs_of(p - b);
      S32 pc = abs_of(p - c);
      // Return nearest of a,b,c
      // breaking ties in order a, b,c
      if (pa <= pb && pa <= pc) {
        paeth_predictor = (U8)a;
      }
      else if (pb <= pc) {
        paeth_predictor = (U8)b;
      }
      else {
        paeth_predictor = (U8)c;
      }
    }
    
    U8 pixel_byte_to_write = (pixel_byte + paeth_predictor)%256;  
    
    srm_write(&cx->image_stream, pixel_byte_to_write);
  }
  return true;
}

static B32
_png_filter_up(_PNG_Context* c) {
  U32 bpl = c->image_width * _PNG_CHANNELS; // bytes per line
  for (U32 i = 0; i < bpl; ++i ){
    U8* pixel_byte_p = srm_consume(U8, &c->unfiltered_image_stream);
    if (pixel_byte_p == null) {
      return false;
    }
    U8 pixel_byte = (*pixel_byte_p); // Up(x)
    
    // NOTE(Momo): Ignore first scanline
    if (c->image_stream.pos < bpl) {
      srm_write(&c->image_stream, pixel_byte);
    }
    else {
      UMI current_index = c->image_stream.pos;
      U8 top = c->image_stream.data[current_index - bpl]; 
      U8 pixel_byte_to_write = (pixel_byte + top) % 256;  
      
      srm_write(&c->image_stream, pixel_byte_to_write);
    }
  }
  
  return true;
}


static B32
_png_filter(_PNG_Context* c) {
  
  srm_reset(&c->unfiltered_image_stream);
  
  // NOTE(Momo): Filter
  // data always starts with 1 byte indicating the type of filter
  // followed by the rest of the chunk.
  U32 counter = 0;
  
  while(!srm_is_eos(&c->unfiltered_image_stream)) {
    U8* filter_type_p = srm_consume(U8, &c->unfiltered_image_stream);
    U8 filter_type = (*filter_type_p);
    // NOTE(Momo): https://www.w3.org/TR/PNG-Filters.html
    switch(filter_type) {
      case 0: { // None
        if (!_png_filter_none(c)) return false;
      } break;
      case 1: { // Sub
        if (!_png_filter_sub(c)) return false;
      } break;
      case 2: {
        if (!_png_filter_up(c)) return false;
      } break;
      case 3: {
        if (!_png_filter_average(c)) return false;
      } break;
      case 4: {
        if (!_png_filter_paeth(c)) return false;
      } break;
      default: {
        return false;
      };
    };
  }
  return true;
  
}

static B32
_png_decompress_zlib(_PNG_Context* c, Stream* zlib_stream) {
  _PNG_IDAT_Header* IDAT = srm_consume(_PNG_IDAT_Header, zlib_stream);
  
  U32 CM = IDAT->compression_flags & 0x0F;
  U32 CINFO = IDAT->compression_flags >> 4;
  U32 FCHECK = IDAT->additional_flags & 0x1F; //not needed?
  U32 FDICT = (IDAT->additional_flags >> 5) & 0x01;
  U32 FLEVEL = (IDAT->additional_flags >> 6); //useless?
  
  
  if (CM != 8 || FDICT != 0 || CINFO > 7) {
    return false;
  }
  
  return _png_deflate(zlib_stream, &c->unfiltered_image_stream, c->allocator);
}


// NOTE(Momo): For the code here, we are going to assume that 
// the PNG file we are reading is correct. i.e. we don't emphasize on 
// checking correctness of the PNG outside of the most basic of checks (e.g. sig)
//
static Image32
png_to_img32(PNG* png, Arena* allocator) 
{
  Image32 ret = {0};
  make(Stream, zlib_stream);

  _PNG_Context ctx = {0};
  ctx.allocator = allocator;
  srm_init(&ctx.stream, png->data, png->data_size);
  ctx.image_width = png->width;
  ctx.image_height = png->height;
  ctx.bit_depth = png->bit_depth;
  
  U32 image_size = png->width * png->height * _PNG_CHANNELS;
  U8* image_stream_memory =  arn_push_arr(U8, allocator, image_size);
  if (!image_stream_memory) goto fail;
  srm_init(&ctx.image_stream, image_stream_memory, image_size);
 
  Arena_Marker mark = arn_mark(allocator);
  //arn_set_revert_point(allocator);
  
  U32 unfiltered_size = png->width * png->height * _PNG_CHANNELS + png->height;
  U8* unfiltered_image_stream_memory = arn_push_arr(U8, allocator, unfiltered_size);
  if (!unfiltered_image_stream_memory) goto fail;
  srm_init(&ctx.unfiltered_image_stream, unfiltered_image_stream_memory, unfiltered_size);
  
  srm_consume(_PNG_Header, &ctx.stream);
  
  // NOTE(Momo): This is really lousy method.
  // We will go through all the IDATs and allocate a giant contiguous 
  // chunk of memory to DEFLATE.
  UMI zlib_size = 0;
  {
    Stream stream = ctx.stream;
    while(!srm_is_eos(&stream)) {
      _PNG_Chunk_Header* chunk_header = srm_consume(_PNG_Chunk_Header, &stream);
      if (!chunk_header) goto fail_and_cleanup;
      U32 chunk_length = endian_swap_u32(chunk_header->length);
      U32 chunk_type = endian_swap_u32(chunk_header->type_U32);
      if (chunk_type == 'IDAT') {
        zlib_size += chunk_length;
      }
      srm_consume_block(&stream, chunk_length);
      srm_consume(_PNG_Chunk_Footer, &stream);
    }
  }
  
  U8* zlib_data = arn_push_arr(U8, allocator, zlib_size);
  if (!zlib_data) goto fail;

  srm_init(zlib_stream, zlib_data, zlib_size);
  
  // Second pass to allocate memory
  while(!srm_is_eos(&ctx.stream)) {
    _PNG_Chunk_Header* chunk_header = srm_consume(_PNG_Chunk_Header, &ctx.stream);
    if (!chunk_header) goto fail_and_cleanup;
    U32 chunk_length = endian_swap_u32(chunk_header->length);
    U32 chunk_type = endian_swap_u32(chunk_header->type_U32);
    if (chunk_type == 'IDAT') {
      srm_write_block(zlib_stream, 
                      ctx.stream.data + ctx.stream.pos,
                      chunk_length);
    }
    srm_consume_block(&ctx.stream, chunk_length);
    srm_consume(_PNG_Chunk_Footer, &ctx.stream);
  }
  srm_reset(zlib_stream);
  
  if (!_png_decompress_zlib(&ctx, zlib_stream)) {
    goto fail_and_cleanup;
  }
  
  if(!_png_filter(&ctx)) {					
    goto fail_and_cleanup;
  }

  ret.width = ctx.image_width;
  ret.height = ctx.image_height;
  ret.pixels = (U32*)ctx.image_stream.data;
  return ret;

fail_and_cleanup:
  arn_revert(mark);
fail:
  return img32_bad();

}


// NOTE(Momo): Really dumb way to write.
// Just have a IHDR, IEND and a single IDAT that's not encoded lul
static Block
png_write_img32_to_blk(Image32 bm, Arena* allocator) {
  if (bm.width <= 0 || bm.height <= 0 || bm.pixels == 0) {
    return blk_bad();
  }
  
  static const U8 signature[] = { 
    137, 80, 78, 71, 13, 10, 26, 10 
  };
  U32 image_bpl = (bm.width * 4);
  U32 data_bpl = image_bpl + 1; // bytes per line
  U32 data_size = data_bpl * bm.height;
  U32 max_chunk_size = 65535;
  U32 signature_size = sizeof(signature);
  U32 chunk_size = sizeof(_PNG_Chunk_Header) + sizeof(_PNG_Chunk_Footer);
  U32 IHDR_size = chunk_size + sizeof(_PNG_IHDR);
  U32 IEND_size = chunk_size;
  U32 IDAT_size = chunk_size + sizeof(_PNG_IDAT_Header);
  U32 lines_per_chunk = max_chunk_size / data_bpl;
  U32 chunk_count = bm.height / lines_per_chunk;
  if (bm.height % lines_per_chunk) {
    chunk_count += 1;
  }
  U32 IDAT_chunk_size = 5 * chunk_count;
  
  U32 expected_memory_required = (signature_size + 
                                  IHDR_size + 
                                  IEND_size + 
                                  IDAT_size + 
                                  data_size + 
                                  IDAT_chunk_size);
  
  U8* stream_memory = arn_push_arr(U8, allocator, expected_memory_required);
  if (!stream_memory) {
    return blk_bad();
  }

  make(Stream, stream);
  srm_init(stream, stream_memory, expected_memory_required);
  srm_write_block(stream, (void*)signature, sizeof(signature));
  
  
  // NOTE(Momo): write IHDR
  {
    U8* crc_start = null;
    
    _PNG_Chunk_Header header = {};
    header.type_U32 = endian_swap_u32('IHDR');
    header.length = sizeof(_PNG_IHDR);
    header.length = endian_swap_u32(header.length);
    srm_write(stream, header);
    crc_start = stream->data + stream->pos - sizeof(header.type_U32);
    
    _PNG_IHDR IHDR = {};
    IHDR.width = endian_swap_u32(bm.width);
    IHDR.height = endian_swap_u32(bm.height);
    IHDR.bit_depth = 8; // ??
    IHDR.colour_type = 6;
    IHDR.compression_method = 0;
    IHDR.filter_method = 0;
    IHDR.interlace_method = 0;
    srm_write(stream, IHDR);
    
    _PNG_Chunk_Footer footer = {};
    U32 crc_size = (U32)(stream->data + stream->pos - crc_start);
    footer.crc = _png_calculate_crc32(crc_start, crc_size); 
    footer.crc = endian_swap_u32(footer.crc);
    srm_write(stream, footer);
    
  }
  
  // NOTE(Momo): write IDAT
  // TODO(Momo): Adler32
  {
    
    U32 chunk_overhead = sizeof(U16)*2 + sizeof(U8)*1;
    
    
    U8* crc_start = null;
    
    _PNG_Chunk_Header header = {};
    header.type_U32 = endian_swap_u32('IDAT');
    header.length = sizeof(_PNG_IDAT_Header) + (chunk_overhead*chunk_count) + data_size; 
    header.length = endian_swap_u32(header.length);    
    srm_write(stream, header);
    crc_start = stream->data + stream->pos - sizeof(header.type_U32);
    
    // NOTE(Momo): Hardcoded IDAT chunk header header that fits our use-case
    //
    // CM = 8
    // CINFO = any number < 7? 1?
    // FCHECK = 23? if CM == 8 and CINFO == 1
    // FDIC = 0;
    // FLEVEL = 1? Documentation says it doesn't matter;
    _PNG_IDAT_Header IDAT;
    IDAT.compression_flags = 8;
    IDAT.additional_flags = 29;
    srm_write(stream, IDAT);
    
    
    // NOTE(Momo): Deflate chunk header
    //
    // BFINAL = 1 (1 bit); // indicates if it's the final block
    // BTYPE = 0 (2 bits); // indicates no compression
    // 
    U32 lines_remaining = bm.height;
    U32 current_line = 0;
    
    for (U32 chunk_index = 0; chunk_index < chunk_count; ++chunk_index){
      U32 lines_to_write = min_of(lines_remaining, lines_per_chunk);
      lines_remaining -= lines_to_write;
      
      U8 BFINAL = ((chunk_index + 1) == chunk_count) ? 1 : 0;
      srm_write(stream, BFINAL);
      
      U16 LEN = (U16)(lines_to_write * data_bpl); // number of data bytes in the block
      U16 NLEN = ~LEN; // one's complement of LEN
      srm_write(stream, LEN);
      srm_write(stream, NLEN);
      
      // NOTE(Momo): Output data here
      // We have to do it row by row to add the filter byte at the front
      for (U32 line_index = 0; line_index < lines_to_write; ++line_index) 
      {
        U8 no_filter = 0;
        srm_write(stream, no_filter); // Filter type: None
        
        srm_write_block(stream,
                        (U8*)bm.pixels + (current_line * image_bpl),
                        image_bpl);
        
        ++current_line;
        
      }
      
      
    }
    
    
    _PNG_Chunk_Footer footer = {};
    U32 crc_size = (U32)(stream->data + stream->pos - crc_start);
    footer.crc = _png_calculate_crc32(crc_start, crc_size); 
    footer.crc = endian_swap_u32(footer.crc);
    srm_write(stream, footer);
  }
  
  // NOTE(Momo): srm_write IEND
  {
    U8* crc_start = null;
    
    _PNG_Chunk_Header header = {};
    header.type_U32 = endian_swap_u32('IEND');
    header.length = 0;
    srm_write(stream, header);
    crc_start = stream->data + stream->pos - sizeof(header.type_U32);
    
    
    _PNG_Chunk_Footer footer = {};
    U32 crc_size = (U32)(stream->data + stream->pos - crc_start);
    footer.crc = _png_calculate_crc32(crc_start, crc_size); 
    footer.crc = endian_swap_u32(footer.crc);
    srm_write(stream, footer);
  }
  
  Block ret;
  ret.data = (U8*)stream->data;
  ret.size = stream->pos;
  
  return ret;
}


static B32
png_read(PNG* p, void* png_memory, UMI png_size) {
  make(Stream, stream);
  srm_init(stream, png_memory, png_size);
  
  // Read Signature
  _PNG_Header* png_header = srm_consume(_PNG_Header, stream);  
  if (!_png_is_signature_valid(png_header->signature)) return false; 
  
  // Read Chunk Header
  _PNG_Chunk_Header* chunk_header = srm_consume(_PNG_Chunk_Header, stream);
  U32 chunk_length = endian_swap_u32(chunk_header->length);
  U32 chunk_type = endian_swap_u32(chunk_header->type_U32);
  
  
  if(chunk_type != 'IHDR') { return false; }
  
  _PNG_IHDR* IHDR = srm_consume(_PNG_IHDR, stream);
  
  // NOTE(Momo): Width and height is in Big Endian
  // We assume that we are currently in a Little Endian system
  U32 width = endian_swap_u32(IHDR->width);
  U32 height = endian_swap_u32(IHDR->height);
  
  if (!_png_is_format_supported(IHDR)) { return false; }
  
  p->data = (U8*)png_memory;
  p->data_size = png_size;
  p->width = width;
  p->height = height;
  p->bit_depth = IHDR->bit_depth;
  p->colour_type = IHDR->colour_type;
  p->compression_method = IHDR->compression_method;
  p->filter_method = IHDR->filter_method;
  p->interlace_method = IHDR->interlace_method;
  
  return true;
}

static B32 
png_read_from_blk(PNG* p, Block blk) {
  return png_read(p, blk.data, blk.size);
}

#endif //MOMO_PNG
