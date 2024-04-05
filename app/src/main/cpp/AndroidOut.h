#ifndef ANDROIDGLINVESTIGATIONS_ANDROIDOUT_H
#define ANDROIDGLINVESTIGATIONS_ANDROIDOUT_H

#include <android/log.h>
#include <sstream>

/*!
 * 使用这个类将字符串记录到logcat中。注意，你应该使用std::endl来提交行。
 *
 * 示例：
 *  aout << "Hello World" << std::endl;
 */
extern std::ostream aout;

/*!
 * 使用这个类创建一个写入logcat的输出流。默认情况下，一个全局实例被定义为 @a aout
 */
class AndroidOut : public std::stringbuf {
public:
    /*!
     * 创建一个新的logcat输出流
     * @param kLogTag 用于输出的log标签
     */
    inline AndroidOut(const char *kLogTag) : logTag_(kLogTag) {}

protected:
    // 当同步输出流时，将字符串内容打印到logcat，并清空字符串缓冲
    virtual int sync() override {
        __android_log_print(ANDROID_LOG_DEBUG, logTag_, "%s", str().c_str());
        str("");
        return 0;
    }

private:
    const char *logTag_; // log标签
};

#endif //ANDROIDGLINVESTIGATIONS_ANDROIDOUT_H
