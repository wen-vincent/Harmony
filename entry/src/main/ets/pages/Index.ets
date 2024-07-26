import { hilog } from '@kit.PerformanceAnalysisKit';
import nativeModule from 'libentry.so'; // 通过import的方式，引入Native能力
import XComponentContext from "../interface/XComponentContext";

const TAG = '[Sample_DrawingAPI]';

@Entry
@Component
struct Index {
  @State message: string = 'Hello World';
  private xComponentContext: XComponentContext | undefined = undefined;

  build() {
    Column() {
      Row() {
        XComponent({ id: 'xcomponentId', type: 'surface', libraryname: 'entry' })
          .onLoad((xComponentContext) => {
            this.xComponentContext = xComponentContext as XComponentContext;
          }).width('640px') // 64的倍数
      }.height('88%')
    }
  }
}
