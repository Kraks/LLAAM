; ModuleID = 'nested_struct.c'
source_filename = "nested_struct.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

%struct.Point = type { i8, i8 }
%struct.Line = type { %struct.Point, %struct.Point }

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca %struct.Point, align 1
  %2 = alloca %struct.Point, align 1
  %3 = alloca %struct.Line, align 1
  %4 = getelementptr inbounds %struct.Point, %struct.Point* %1, i32 0, i32 0
  store i8 2, i8* %4, align 1
  %5 = getelementptr inbounds %struct.Point, %struct.Point* %1, i32 0, i32 1
  store i8 3, i8* %5, align 1
  %6 = getelementptr inbounds %struct.Point, %struct.Point* %2, i32 0, i32 0
  store i8 5, i8* %6, align 1
  %7 = getelementptr inbounds %struct.Point, %struct.Point* %2, i32 0, i32 1
  store i8 6, i8* %7, align 1
  %8 = getelementptr inbounds %struct.Line, %struct.Line* %3, i32 0, i32 0
  %9 = bitcast %struct.Point* %8 to i8*
  %10 = bitcast %struct.Point* %1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %9, i8* %10, i64 2, i32 1, i1 false)
  %11 = getelementptr inbounds %struct.Line, %struct.Line* %3, i32 0, i32 1
  %12 = bitcast %struct.Point* %11 to i8*
  %13 = bitcast %struct.Point* %2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %12, i8* %13, i64 2, i32 1, i1 false)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
