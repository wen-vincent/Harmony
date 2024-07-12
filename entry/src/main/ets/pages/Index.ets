import { hilog } from '@kit.PerformanceAnalysisKit';
import nativeModule from 'libentry.so'; // 通过import的方式，引入Native能力

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(50)
          .fontWeight(FontWeight.Bold)
          .onClick( () => {
            let callback = (parm:string): Promise<string> => {
              hilog.debug(0x1234,"mytest","-------------"+parm);
            return new Promise((resolve) => {
              setTimeout(() => {
                hilog.debug(0x1234,"mytest","reasdasd");
                  resolve("xxxxxxxxxxxxxxxxxxxxxx");
                }, 1000);
              });
            }
            hilog.debug(0x1234,"mytest","star");
            nativeModule.startThread(callback);
            hilog.debug(0x1234,"mytest","stop");
            })
      }
      .width('100%')
    }
    .height('100%')
  }
}
