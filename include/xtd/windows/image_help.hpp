#pragma once


namespace xtd {
  namespace windows {
    namespace image_help {


      extern "C" IMAGE_DOS_HEADER __ImageBase;

      struct data_directory : private IMAGE_DATA_DIRECTORY {
      private:
        friend struct binary;
      };

      struct file_header : private IMAGE_FILE_HEADER {
      private:
        friend struct binary;
      };

      struct optional_header : private IMAGE_OPTIONAL_HEADER {
      private:
        friend struct binary;
        friend struct nt_header;
      };

      struct section_header : private IMAGE_SECTION_HEADER{
      private:
        friend struct binary;
      };

      struct nt_header : private IMAGE_NT_HEADERS {

        const image_help::file_header& file_header() const { return *reinterpret_cast<const image_help::file_header*>(&FileHeader); }

        const image_help::optional_header& optional_header() const {
          auto image_optional_header = reinterpret_cast<const image_help::optional_header*>(&OptionalHeader);
          xtd::exception::throw_if(image_optional_header->Magic, [](WORD sig) { return IMAGE_NT_OPTIONAL_HDR32_MAGIC != sig  && IMAGE_NT_OPTIONAL_HDR64_MAGIC != sig && IMAGE_ROM_OPTIONAL_HDR_MAGIC != sig; });
          return *image_optional_header;
        }

      private:
        friend struct binary;
      };

      struct binary : private IMAGE_DOS_HEADER{

        static const binary& this_binary() {
          auto & dos_image_header = reinterpret_cast<binary&>(__ImageBase);
          xtd::exception::throw_if(dos_image_header.e_magic, [](WORD sig) {return IMAGE_DOS_SIGNATURE != sig; });
          return dos_image_header;
        }

        WORD checksum() const { return e_csum; }

        const image_help::nt_header& nt_header() const {
          auto nt_image_header = reinterpret_cast<image_help::nt_header*>(e_lfanew + reinterpret_cast<size_t>(this));
          xtd::exception::throw_if(nt_image_header->Signature, [](DWORD sig) { return IMAGE_NT_SIGNATURE != sig; });
          return *nt_image_header;
        }

        const image_help::section_header& section_header() const {
          return *reinterpret_cast<image_help::section_header*>(e_lfanew + sizeof(IMAGE_NT_HEADERS) + reinterpret_cast<size_t>(this));
        }
        
      private:

      };

    }
  }
}


int main() {
  auto & oBinary = xtd::windows::image_help::binary::this_binary();
  auto & oSect = oBinary.section_header();
  auto & oNTHeader = oBinary.nt_header();
  auto & oOptional = oNTHeader.optional_header();
  return oBinary.checksum();
}