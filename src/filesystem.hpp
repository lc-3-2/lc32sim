#pragma once

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "iodevice.hpp"
#include "memory.hpp"
#include "utils.hpp"
#include "lc32_flags.hpp"
#include "log.hpp"

namespace lc32sim { 
    using sim_fd = uint16_t;
    using sim_size_t = uint32_t;
    using sim_long = int64_t;
    using sim_int = int32_t;

    // this guarantee is needed for safe conversions
    static_assert(sizeof(sim_size_t) <= sizeof(size_t));

    class Filesystem : public IODevice {
        private:
            struct File {
                int fd = 0;
                bool open = false;
                File(int fd, bool open) : fd(fd), open(open) {}
            };

            std::vector<File> file_table;
            Memory &mem;

            std::unordered_map<int, int> flags_map;
            std::unordered_map<int, int> errno_map;
            std::unordered_map<int, int> whence_map;

            static const uint16_t MODE_OFF = 0;
            static const uint16_t MODE_OPEN = 1;
            static const uint16_t MODE_CLOSE = 2;
            static const uint16_t MODE_READ = 3;
            static const uint16_t MODE_WRITE = 4;
            static const uint16_t MODE_SEEK = 5;
            static const uint16_t MODE_FSTAT = 6;
            static const uint16_t MODE_ISATTY = 7;
            static const uint16_t MODE_LINK = 8;
            static const uint16_t MODE_UNLINK = 9;
            static const uint16_t MODE_MKDIR = 10;


        public:
            Filesystem(Memory &mem) : file_table(), mem(mem), flags_map(), errno_map(), whence_map() {
                flags_map[LC32_O_RDONLY] = O_RDONLY;
                flags_map[LC32_O_WRONLY] = O_WRONLY;
                flags_map[LC32_O_RDWR] = O_RDWR;
                flags_map[LC32_O_APPEND] = O_APPEND;
                flags_map[LC32_O_CREAT] = O_CREAT;
                flags_map[LC32_O_TRUNC] = O_TRUNC;

                whence_map[LC32_SEEK_CUR] = SEEK_CUR;
                whence_map[LC32_SEEK_SET] = SEEK_SET;
                whence_map[LC32_SEEK_END] = SEEK_END;

                // populate errno map
                errno_map[LC32_EPERM] = EPERM;                              
                errno_map[LC32_ENOENT] = ENOENT;                            
                errno_map[LC32_ESRCH] = ESRCH;                              
                errno_map[LC32_EINTR] = EINTR;                              		
                errno_map[LC32_EIO] = EIO;                                		
                errno_map[LC32_ENXIO] = ENXIO;                              		
                errno_map[LC32_E2BIG] = E2BIG;                              		
                errno_map[LC32_ENOEXEC] = ENOEXEC;                            	
                errno_map[LC32_EBADF] = EBADF;                              
                errno_map[LC32_ECHILD] = ECHILD;                            	
                errno_map[LC32_EAGAIN] = EAGAIN;                            	
                errno_map[LC32_ENOMEM] = ENOMEM;                            	
                errno_map[LC32_EACCES] = EACCES;                            	
                errno_map[LC32_EFAULT] = EFAULT;                            	
                errno_map[LC32_EBUSY] = EBUSY;                              	
                errno_map[LC32_EEXIST] = EEXIST;                            	
                errno_map[LC32_EXDEV] = EXDEV;                              	
                errno_map[LC32_ENODEV] = ENODEV;                            	
                errno_map[LC32_ENOTDIR] = ENOTDIR;                           	
                errno_map[LC32_EISDIR] = EISDIR;                             	
                errno_map[LC32_EINVAL] = EINVAL;                            
                errno_map[LC32_ENFILE] = ENFILE;                             	
                errno_map[LC32_EMFILE] = EMFILE;                             	
                errno_map[LC32_ENOTTY] = ENOTTY;                             	
                errno_map[LC32_ETXTBSY] = ETXTBSY;                           	
                errno_map[LC32_EFBIG] = EFBIG;                              	
                errno_map[LC32_ENOSPC] = ENOSPC;                             	
                errno_map[LC32_ESPIPE] = ESPIPE;                             	
                errno_map[LC32_EROFS] = EROFS;                              	
                errno_map[LC32_EMLINK] = EMLINK;                             	
                errno_map[LC32_EPIPE] = EPIPE;                              	
                errno_map[LC32_EDOM] = EDOM;                               		
                errno_map[LC32_ERANGE] = ERANGE;                            	
                errno_map[LC32_ENOMSG] = ENOMSG;                            	
                errno_map[LC32_EIDRM] = EIDRM;                              	
                errno_map[LC32_EDEADLK] = EDEADLK;                           	
                errno_map[LC32_ENOLCK] = ENOLCK;                             	
                errno_map[LC32_ENOSTR] = ENOSTR;                             	
                errno_map[LC32_ENODATA] = ENODATA;                           	
                errno_map[LC32_ETIME] = ETIME;                              	
                errno_map[LC32_ENOSR] = ENOSR;                              	
                errno_map[LC32_ENOLINK] = ENOLINK;                           
                errno_map[LC32_EPROTO] = EPROTO;                             	
                errno_map[LC32_EMULTIHOP] = EMULTIHOP;                          
                errno_map[LC32_EBADMSG] = EBADMSG;                            	
                errno_map[LC32_ENOSYS] = ENOSYS;                             	
                errno_map[LC32_ENOTEMPTY] = ENOTEMPTY;                         
                errno_map[LC32_ENAMETOOLONG] = ENAMETOOLONG;                      
                errno_map[LC32_ELOOP] = ELOOP;                             
                errno_map[LC32_EOPNOTSUPP] = EOPNOTSUPP;                        
                errno_map[LC32_EPFNOSUPPORT] = EPFNOSUPPORT;                      
                errno_map[LC32_ECONNRESET] = ECONNRESET;                        
                errno_map[LC32_ENOBUFS] = ENOBUFS;                           
                errno_map[LC32_EAFNOSUPPORT] = EAFNOSUPPORT;                      
                errno_map[LC32_EPROTOTYPE] = EPROTOTYPE;                        
                errno_map[LC32_ENOTSOCK] = ENOTSOCK;                           
                errno_map[LC32_ENOPROTOOPT] = ENOPROTOOPT;                        
                errno_map[LC32_ECONNREFUSED] = ECONNREFUSED;                      
                errno_map[LC32_EADDRINUSE] = EADDRINUSE;                         
                errno_map[LC32_ECONNABORTED] = ECONNABORTED;                      
                errno_map[LC32_ENETUNREACH] = ENETUNREACH;                       
                errno_map[LC32_ENETDOWN] = ENETDOWN;                           
                errno_map[LC32_ETIMEDOUT] = ETIMEDOUT;                         
                errno_map[LC32_EHOSTDOWN] = EHOSTDOWN;                         
                errno_map[LC32_EHOSTUNREACH] = EHOSTUNREACH;                      
                errno_map[LC32_EINPROGRESS] = EINPROGRESS;                        		
                errno_map[LC32_EALREADY] = EALREADY;                           		
                errno_map[LC32_EDESTADDRREQ] = EDESTADDRREQ;                       	
                errno_map[LC32_EMSGSIZE] = EMSGSIZE;                           		
                errno_map[LC32_EPROTONOSUPPORT] = EPROTONOSUPPORT;                    	
                errno_map[LC32_EADDRNOTAVAIL] = EADDRNOTAVAIL;                      
                errno_map[LC32_ENETRESET] = ENETRESET;                          	
                errno_map[LC32_EISCONN] = EISCONN;                            		
                errno_map[LC32_ENOTCONN] = ENOTCONN;                           	
                errno_map[LC32_ETOOMANYREFS] = ETOOMANYREFS;                       
                errno_map[LC32_EDQUOT] = EDQUOT;                             
                errno_map[LC32_ESTALE] = ESTALE;                             
                errno_map[LC32_ENOTSUP] = ENOTSUP;                            		
                errno_map[LC32_EILSEQ] = EILSEQ;                             		
                errno_map[LC32_EOVERFLOW] = EOVERFLOW;                         
                errno_map[LC32_ECANCELED] = ECANCELED;                          	
                errno_map[LC32_ENOTRECOVERABLE] = ENOTRECOVERABLE;                    
                errno_map[LC32_EOWNERDEAD] = EOWNERDEAD;                         	

                // Add stdout, stderr, and stdin to table
                file_table.insert(file_table.begin() + LC32_STDIN_FILENO, File(STDIN_FILENO, true));
                file_table.insert(file_table.begin() + LC32_STDOUT_FILENO, File(STDOUT_FILENO, true));
                file_table.insert(file_table.begin() + LC32_STDERR_FILENO, File(STDERR_FILENO, true));
            }

            sim_fd sim_open(const char *filename, int flags, int mode, sim_int *error);
            sim_size_t sim_read(sim_fd fd, void *ptr, sim_size_t cnt, sim_int *error);
            sim_size_t sim_write(sim_fd fd, const void *ptr, sim_size_t cnt, sim_int *error);
            sim_size_t sim_seek(sim_fd fd, sim_int offset, sim_int whence, sim_int *error);
            sim_int sim_close(sim_fd fd, sim_int *error);
            sim_int sim_fstat(sim_fd fd, sim_int pstat, sim_int *error);
            sim_int sim_isatty(sim_fd fd, sim_int *error);
            sim_int sim_link(const char *oldpath, const char *newpath, sim_int *error);
            sim_int sim_unlink(const char *pathname, sim_int *error);
            sim_int sim_mkdir(const char *pathname, sim_int mode, sim_int *error);

            int convert_flags(sim_int flags);
            sim_int convert_errno(int sim_errno);
            int convert_whence(sim_int whence);
            bool copy_stat(sim_int plc32_stat, struct stat local_stat);
            bool will_overflow(sim_int num_bits, long num);


            std::string get_name() override { return "Filesystem"; };
            write_handlers get_write_handlers() override {
                return {
                    { FS_CONTROLLER_ADDR, [this](uint32_t old_value, uint32_t value) {
                        static_assert(sizeof(sim_fd) == sizeof(uint16_t));
                        uint16_t mode = first16(value);
                        sim_fd fd = second16(value);
                        sim_int error = 0;

                        if (logger.info.enabled()) {
                            logger.info << "Filesystem peripheral handler set off with mode: " << mode;
                        }

                        if (mode == MODE_OFF) {
                            return value;
                        }

                        uint32_t data1 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 4);
                        uint32_t data2 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 8);
                        uint32_t data3 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 12);
                        uint64_t data12;

                        uint32_t ret;
                        switch (mode) {
                            case MODE_OPEN:
                                ret = this->sim_open(this->mem.ptr_to<const char>(data1), data2, data3, &error);
                                this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, error);
                                return from16(MODE_OFF, ret);
                            case MODE_CLOSE:
                                ret = this->sim_close(fd, &error);
                                goto write_ret;
                            case MODE_READ:
                                ret = this->sim_read(fd, this->mem.ptr_to<void>(data1), data2, &error);
                                goto write_ret;
                            case MODE_WRITE:
                                ret = this->sim_write(fd, this->mem.ptr_to<const void>(data1), data2, &error);
                                goto write_ret;
                            case MODE_SEEK:
                                data12 = ((static_cast<uint64_t>(data2) << 32) | data1);
                                ret = this->sim_seek(fd, data1, data2, &error);
                                goto write_ret;
                            case MODE_FSTAT:
                                ret = this->sim_fstat(fd, data1, &error);
                                goto write_ret;
                            case MODE_ISATTY:
                                ret = this->sim_isatty(fd, &error);
                                goto write_ret;
                            case MODE_LINK:
                                ret = this->sim_link(this->mem.ptr_to<const char>(data1), this->mem.ptr_to<const char>(data2), &error);
                                goto write_ret;
                            case MODE_UNLINK:
                                ret = this->sim_unlink(this->mem.ptr_to<const char>(data1), &error);
                                goto write_ret;
                            case MODE_MKDIR:
                                ret = this->sim_mkdir(this->mem.ptr_to<const char>(data1), data2, &error);
                                goto write_ret;
                            write_ret:
                                this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, error);
                                this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 12, ret);
                                break;
                        }

                        return from16(MODE_OFF, second16(value));
                    }
                }};
            }
    };
}