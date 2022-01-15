
static Mailbox
CreateMailbox(void* memory, UMI memory_size) {
  Mailbox ret;
  ret.memory = (U8*)memory;
  ret.memory_size = memory_size;
  
  clear(&ret);
  
	return ret;
}

static void
clear(Mailbox* m) {
  m->data_pos = 0;	
	m->entry_count = 0;
	
	UMI imem = PtrToInt(m->memory);
	UMI adjusted_entry_start = AlignDownPow2(imem + m->memory_size, 4) - imem;
	
	m->entry_start = m->entry_pos = adjusted_entry_start;
	
}


// NOTE(Momo): Accessors and Iterators
static Mailbox_Entry*
GetEntry(Mailbox* m, UMI index) {
  Assert(index < m->entry_count);
	
	UMI stride = AlignUpPow2(sizeof(Mailbox_Entry), 4);
	return (Mailbox_Entry*)(m->memory + m->entry_start - ((index+1) * stride));
}



static void*
make_block(Mailbox* m, UMI size, U32 id, UMI align) 
{
	UMI imem = PtrToInt(m->memory);
	
	UMI adjusted_data_pos = AlignUpPow2(imem + m->data_pos, align) - imem;
	UMI adjusted_entry_pos = AlignDownPow2(imem + m->entry_pos, 4) - imem; 
	
	Assert(adjusted_data_pos + size + sizeof(Mailbox_Entry) < adjusted_entry_pos);
	
	m->data_pos = adjusted_data_pos + size;
	m->entry_pos = adjusted_entry_pos - sizeof(Mailbox_Entry);
	
	Mailbox_Entry* entry = (Mailbox_Entry*)IntToPtr(imem + m->entry_pos);
	entry->id = id;
	entry->data = IntToPtr(imem + adjusted_data_pos);
	
	
	++m->entry_count;
	
	return entry->data;
}


static void* 
PushExtraData(Mailbox* m, UMI size, UMI align)
{
  UMI imem = PtrToInt(m->memory);
  UMI adjusted_data_pos = AlignUpPow2(imem + m->data_pos, align) - imem;
  
  Assert(adjusted_data_pos + size < m->entry_pos);
  
  m->data_pos = adjusted_data_pos + size;
  
  return IntToPtr(imem + adjusted_data_pos);
  
}

template<typename T> static T*	  
Push(Mailbox* m, U32 id, UMI align) {
  return (T*)make_block(m, sizeof(T), id, align);
}  
