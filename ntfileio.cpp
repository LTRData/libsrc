#ifndef _DLL
#define _DLL
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ntdll.h>
#include <intsafe.h>
#include <winstrct.h>

#include <ntfileio.hpp>

#ifndef USHORT_MAX
#define USHORT_MAX INTSAFE_USHORT_MAX
#endif

HANDLE
NativeCreateDirectory(IN HANDLE RootDirectory,
		      IN PUNICODE_STRING Name,
		      IN ACCESS_MASK DesiredAccess,
		      IN ULONG ObjectOpenAttributes,
		      OUT PULONG_PTR CreationResult OPTIONAL,
		      IN ULONG FileAttributes,
		      IN ULONG ShareAccess,
		      IN ULONG CreateOptions,
		      IN BOOL CreateParentDirectories)
{
  // If "File" is current directory marker ".", we need to pass just
  // root directory and an empty name to directory create routines
  UNICODE_STRING name = { 0 };
  if ((Name->Length != 2) ? true : (Name->Buffer[0] != L'.'))
    name = *Name;

  OBJECT_ATTRIBUTES object_attributes;

  InitializeObjectAttributes(&object_attributes,
			     &name,
			     ObjectOpenAttributes,
			     RootDirectory,
			     NULL);

  IO_STATUS_BLOCK io_status;
  HANDLE handle;

  for (;;)
    {
      NTSTATUS status =
	NtCreateFile(&handle,
		     DesiredAccess | SYNCHRONIZE,
		     &object_attributes,
		     &io_status,
		     NULL,
		     FileAttributes,
		     ShareAccess,
		     FILE_OPEN_IF,
		     CreateOptions |
		     FILE_DIRECTORY_FILE |
		     FILE_SYNCHRONOUS_IO_NONALERT,
		     NULL,
		     0);

      if (NT_SUCCESS(status))
	{
	  if (CreationResult != NULL)
	    *CreationResult = io_status.Information;

	  return handle;
	}

      if (CreateParentDirectories &&
	  (status == STATUS_OBJECT_PATH_NOT_FOUND))
	{
	  UNICODE_STRING parent_dir;
	  SplitPath(Name, &parent_dir, NULL);

	  if (parent_dir.Length > 0)
	    {
	      parent_dir.Length -= 2;
	      ULONG_PTR ParentCreationResult;
	      HANDLE parent_dir_handle =
		NativeCreateDirectory(RootDirectory,
				      &parent_dir,
				      FILE_LIST_DIRECTORY | FILE_TRAVERSE,
				      ObjectOpenAttributes,
				      &ParentCreationResult,
				      FILE_ATTRIBUTE_NORMAL,
				      FILE_SHARE_READ | FILE_SHARE_WRITE |
				      FILE_SHARE_DELETE,
				      0,
				      TRUE);
	      if (parent_dir_handle != INVALID_HANDLE_VALUE)
		{
		  NtClose(parent_dir_handle);

		  if (ParentCreationResult == FILE_CREATED)
		    continue;
		}
	    }
	}

      SetLastError(RtlNtStatusToDosError(status));
      return INVALID_HANDLE_VALUE;
    }
}

HANDLE
NativeCreateFile(IN HANDLE RootDirectory,
		 IN PUNICODE_STRING Name,
		 IN ACCESS_MASK DesiredAccess,
		 IN ULONG ObjectOpenAttributes,
		 OUT PULONG_PTR CreationResult OPTIONAL,
		 IN PLARGE_INTEGER AllocationSize OPTIONAL,
		 IN ULONG FileAttributes,
		 IN ULONG ShareAccess,
		 IN ULONG CreateDisposition,
		 IN ULONG CreateOptions,
		 IN BOOL CreateParentDirectories)
{
  OBJECT_ATTRIBUTES object_attributes;

  InitializeObjectAttributes(&object_attributes,
			     Name,
			     ObjectOpenAttributes,
			     RootDirectory,
			     NULL);

  IO_STATUS_BLOCK io_status;
  HANDLE handle;

  for (;;)
    {
      NTSTATUS status =
	NtCreateFile(&handle,
		     DesiredAccess | SYNCHRONIZE,
		     &object_attributes,
		     &io_status,
		     AllocationSize,
		     FileAttributes,
		     ShareAccess,
		     CreateDisposition,
		     CreateOptions |
		     FILE_NON_DIRECTORY_FILE |
		     FILE_SYNCHRONOUS_IO_NONALERT,
		     NULL,
		     0);

      if (NT_SUCCESS(status))
	{
	  if (CreationResult != NULL)
	    *CreationResult = io_status.Information;

	  return handle;
	}

      if (CreateParentDirectories &&
	  (status == STATUS_OBJECT_PATH_NOT_FOUND))
	{
	  UNICODE_STRING parent_dir = *Name;

	  for (;
	       parent_dir.Length > 0;
	       parent_dir.Length -= 2)
	    if (parent_dir.Buffer[(parent_dir.Length >> 1) - 1] == L'\\')
	      break;

	  if (parent_dir.Length > 0)
	    {
	      parent_dir.Length -= 2;
	      ULONG_PTR ParentCreationResult;
	      HANDLE parent_dir_handle =
		NativeCreateDirectory(RootDirectory,
				      &parent_dir,
				      FILE_LIST_DIRECTORY | FILE_TRAVERSE,
				      ObjectOpenAttributes,
				      &ParentCreationResult,
				      FILE_ATTRIBUTE_NORMAL,
				      FILE_SHARE_READ | FILE_SHARE_WRITE |
				      FILE_SHARE_DELETE,
				      0,
				      TRUE);
	      if (parent_dir_handle != INVALID_HANDLE_VALUE)
		{
		  NtClose(parent_dir_handle);

		  if (ParentCreationResult == FILE_CREATED)
		    continue;
		}
	    }
	}

      SetLastError(RtlNtStatusToDosError(status));
      return INVALID_HANDLE_VALUE;
    }
}
