; ModuleID = 'multi_array.c'
source_filename = "multi_array.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
  %1 = alloca [3 x [4 x i8]], align 1
  %2 = alloca [3 x [4 x [5 x i8]]], align 16
  %3 = getelementptr inbounds [3 x [4 x i8]], [3 x [4 x i8]]* %1, i64 0, i64 1
  %4 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 3
  store i8 3, i8* %4, align 1
  %5 = getelementptr inbounds [3 x [4 x [5 x i8]]], [3 x [4 x [5 x i8]]]* %2, i64 0, i64 2
  %6 = getelementptr inbounds [4 x [5 x i8]], [4 x [5 x i8]]* %5, i64 0, i64 3
  %7 = getelementptr inbounds [5 x i8], [5 x i8]* %6, i64 0, i64 4
  store i8 5, i8* %7, align 1
  ret i32 0
}

attributes #0 = { nounwind ssp uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.0.0 (clang-800.0.42.1)"}
