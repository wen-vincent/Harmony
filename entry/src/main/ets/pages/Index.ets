import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';

async function delay(ms:number):Promise<number>{
  return new Promise(resolve => setTimeout(resolve, ms));
}

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
            // hilog.info(0x0000, 'testTag', 'Test NAPI 2 + 3 = %{public}d', testNapi.add(2, 3));
            testNapi.asyncWork(10, 22,  (result) => {
              hilog.info(0x0000, 'mytest', 'result is %{public}d', result);
              // await delay(1000);
              return 12321;
            });
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
