# Hexa 語言解釋器擴展報告

## 1. 引言

本報告旨在詳細說明對 Hexa 語言解釋器進行的擴展工作。根據使用者需求，本次擴展的核心目標是完善並補全一個真正圖靈完備、支持遞迴與閉包的 Hexa 解釋器，確保其能夠正確執行如 factorial、Fibonacci 等操作，並具備現代程式語言應有的核心特性。

## 2. 主要變更與實現

為達成上述目標，我們對 Hexa 解釋器的核心組件進行了以下主要修改與增強：

### 2.1. 閉包 (Closure) 的實現

閉包是本次擴展的核心。我們進行了以下工作以完整支援詞法作用域閉包：

*   **環境捕獲**: 修改了 `Function` 資料結構 (`include/hexa.h`)，增加了一個 `captured_env` 指標，用於儲存函數定義時的環境。當定義一個函數時 (`evaluator.c` 中的 `defineFn`)，當前的環境會被捕獲並儲存。
*   **函數呼叫**: 修改了函數呼叫的求值邏輯 (`evaluator.c` 中的 `evaluateList`)。當呼叫一個使用者定義的函數時，會創建一個新的執行環境，其外層環境 (enclosing environment) 被設置為該函數的 `captured_env`，而非呼叫時的當前環境。這確保了函數能夠存取其定義時作用域內的變數，即使定義時的作用域已經結束。
*   **環境記憶體管理 (引用計數)**: 為了正確管理多層閉包和高階函數場景下環境的生命週期，防止記憶體洩漏或懸空指標，我們引入了引用計數 (reference counting) 機制來管理 `Environment` 物件：
    *   `Environment` 結構 (`include/hexa.h`) 中增加了 `ref_count` 欄位。
    *   `createEnvironment` (`environment.c`) 初始化 `ref_count` 為 1。
    *   `createEnclosedEnvironment` (`environment.c`) 在創建內部環境時，會遞增其外層 (enclosing) 環境的 `ref_count`。
    *   當函數捕獲環境時 (`evaluator.c` 的 `defineFn`)，被捕獲環境的 `ref_count` 會遞增。
    *   當複製一個 `Function` 值時 (`value.c` 的 `copyValue`)，其 `captured_env` 的 `ref_count` 會遞增。
    *   引入了 `releaseEnvironmentReference` 函數 (`environment.c`) 來取代原有的 `freeEnvironment`。此函數會遞減環境的 `ref_count`，只有當 `ref_count` 降至 0 時，才會真正釋放環境及其包含的資源（包括遞迴釋放其外層環境的引用）。
    *   所有先前直接呼叫 `freeEnvironment` 的地方（例如 `evaluator.c` 中函數執行完畢後釋放執行環境，`value.c` 中釋放 `Function` 值時釋放其捕獲的環境，以及 `main.c` 中程式結束時釋放全域環境）均已更新為呼叫 `releaseEnvironmentReference`。

### 2.2. 遞迴 (Recursion)

透過正確實現詞法作用域閉包，遞迴函數的支援得到了根本性的保障。函數在遞迴呼叫自身時，能夠正確地在其定義時的環境中查找變數（包括函數自身的名字），從而實現正確的遞迴行為。

### 2.3. 圖靈完備性 (Turing Completeness)

解釋器現已具備以下核心要素，確保了其圖靈完備性：

*   **條件分支**: `if` 特殊形式允許根據條件執行不同的程式碼路徑。
*   **變數定義與修改**: `def` 特殊形式允許定義變數，並可在其作用域內被存取與（間接透過函數呼叫）修改。
*   **函數定義與呼叫**: `fn` 特殊形式允許定義函數，函數可以作為一等公民被傳遞、返回和遞迴呼叫。
*   **基本運算**: 提供了算術運算、比較運算等基礎操作。
*   **序列執行**: 列表中的表達式會被順序求值。

### 2.4. 記憶體管理與穩定性修正

除了閉包環境的引用計數外，還進行了以下修正以提升解釋器的穩定性和記憶體安全性：

*   **字串與符號的複製**: 在 `value.c` 和 `environment.c` 中，原先部分使用 `strdup` 的地方（在某些環境下可能未定義或行為不一致）已改為使用 `malloc` 和 `strcpy`，並增加了必要的錯誤處理和標頭檔引用 (`string.h`)。
*   **`Value` 的複製與釋放**: `copyValue` 和 `freeValue` (`value.c`) 函數已更新，以正確處理 `VAL_FUNCTION` 類型的值，確保其 `captured_env` 的引用計數得到妥善管理。
*   **編譯錯誤與警告修正**: 解決了因模組間依賴關係調整（如 `initGlobalEnvironment` 的位置）導致的編譯錯誤，並修正了部分編譯警告。

## 3. 測試與驗證

我們進行了全面的測試以驗證擴展功能的正確性和解釋器的穩定性：

*   **基礎測試 (`examples/hello.hexa`)**: 此檔案包含了基本的輸出、變數定義、以及 factorial 和 Fibonacci 遞迴函數的實現。測試結果表明，這些基礎功能和遞迴操作均能正確執行。
*   **進階閉包測試 (`examples/closure_tests.hexa`)**: 我們編寫了新的測試案例，專門用於驗證閉包的各種複雜場景，包括：
    *   函數作為返回值（工廠函數模式）。
    *   函數作為參數傳遞（高階函數模式）。
    *   多層嵌套閉包，驗證自由變數的正確捕獲與跨層級存取。
    所有進階閉包測試均已通過，證明了解釋器對詞法作用域閉包的支援是完整和正確的。

經過上述修改和測試，Hexa 解釋器現在能夠穩定、正確地執行包含複雜遞迴和閉包結構的程式碼。

## 4. 如何編譯與執行

1.  **環境準備**: 確保系統已安裝 C 編譯器 (如 GCC) 和 `make` 工具。
2.  **獲取原始碼**: 從提供的附件中解壓縮 `Hexa-Language` 目錄。
3.  **編譯**: 在 `Hexa-Language` 目錄下，執行 `make` 命令：
    ```bash
    make clean && make
    ```
    這將清除舊的建置檔案並重新編譯解釋器，生成名為 `hexai` 的可執行檔。
4.  **執行**: 
    *   **執行 Hexa 檔案**: `./hexai examples/your_file.hexa`
    *   **進入 REPL (互動模式)**: `./hexai`

## 5. 結論

本次擴展成功地將 Hexa 解釋器提升為一個支援圖靈完備操作、遞迴以及現代詞法作用域閉包的程式語言工具。透過引入引用計數機制管理環境生命週期，解決了先前在複雜閉包場景下可能出現的記憶體問題，確保了解釋器的穩定性和正確性。Hexa 語言現已具備更強大的表達能力和更廣泛的應用潛力。

---

**附錄：主要修改檔案列表**

*   `include/hexa.h`: 修改了 `Function` 和 `Environment` 結構，更新了函數原型。
*   `src/environment.c`: 實現了環境的引用計數管理 (`ref_count`, `releaseEnvironmentReference`, `trulyFreeEnvironment`)，修改了變數定義時的字串複製邏輯。
*   `src/evaluator.c`: 修改了函數定義 (`defineFn`) 以捕獲環境並遞增 `ref_count`，修改了函數呼叫 (`evaluateList`) 以使用捕獲的環境創建執行環境，並將環境釋放改為 `releaseEnvironmentReference`。
*   `src/value.c`: 修改了 `makeFunction` 初始化 `captured_env`，修改了 `freeValue` 在釋放 `VAL_FUNCTION` 時呼叫 `releaseEnvironmentReference`，修改了 `copyValue` 在複製 `VAL_FUNCTION` 時遞增 `captured_env` 的 `ref_count`，修正了 `makeString` 和 `makeSymbol` 的字串複製邏輯。
*   `src/main.c`: 修改了程式結束時釋放全域環境的方式為 `releaseEnvironmentReference`。
*   `Makefile`: 無顯著修改，但確保能正確編譯所有更新後的檔案。
*   `examples/closure_tests.hexa`: 新增的閉包測試案例檔案。

