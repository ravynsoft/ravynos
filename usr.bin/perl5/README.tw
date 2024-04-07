# vim: syntax=pod

如果你用一般的文字編輯器閱覽這份文件, 請忽略文中奇特的註記字符.
這份文件是以 POD (簡明文件格式) 寫成; 這種格式是為了能讓人直接讀取,
而特別設計的. 關於此格式的進一步資訊, 請參考 perlpod 線上文件.

=encoding utf8

=head1 NAME

perltw - 正體中文 Perl 指南

=head1 DESCRIPTION

歡迎來到 Perl 的天地!

從 5.8.0 版開始, Perl 具備了完善的 Unicode (萬國碼) 支援,
也連帶支援了許多拉丁語系以外的編碼方式; CJK (中日韓) 便是其中的一部份.
Unicode 是國際性的標準, 試圖涵蓋世界上所有的字符: 西方世界, 東方世界,
以及兩者間的一切 (希臘文, 敘利亞文, 阿拉伯文, 希伯來文, 印度文,
印地安文, 等等). 它也容納了多種作業系統與平臺 (如 PC 及麥金塔).

Perl 本身以 Unicode 進行操作. 這表示 Perl 內部的字串資料可用 Unicode
表示; Perl 的函式與算符 (例如正規表示式比對) 也能對 Unicode 進行操作.
在輸入及輸出時, 為了處理以 Unicode 之前的編碼方式儲存的資料, Perl
提供了 Encode 這個模組, 可以讓你輕易地讀取及寫入舊有的編碼資料.

Encode 延伸模組支援下列正體中文的編碼方式 ('big5' 表示 'big5-eten'):

    big5-eten	Big5 編碼 (含倚天延伸字形)
    big5-hkscs	Big5 + 香港外字集, 2001 年版
    cp950	字碼頁 950 (Big5 + 微軟添加的字符)

舉例來說, 將 Big5 編碼的檔案轉成 Unicode, 祗需鍵入下列指令:

    perl -MEncode -pe '$_= encode( utf8 => decode( big5 => $_ ) )' \
      < file.big5 > file.utf8

Perl 也內附了 "piconv", 一支完全以 Perl 寫成的字符轉換工具程式, 用法如下:

    piconv -f big5 -t utf8 < file.big5 > file.utf8
    piconv -f utf8 -t big5 < file.utf8 > file.big5

另外，若程式碼本身以 utf8 編碼儲存，配合使用 utf8 模組，可讓程式碼中字串以及其運
算皆以字符為單位，而不以位元為單位，如下所示：

    #!/usr/bin/env perl
    use utf8;
    print length("駱駝");	     #  2 (不是 6)
    print index("諄諄教誨", "教誨"); #  2 (從 0 起算第 2 個字符)


=head2 額外的中文編碼

如果需要更多的中文編碼, 可以從 CPAN (L<https://www.cpan.org/>) 下載
Encode::HanExtra 模組. 它目前提供下列編碼方式:

    cccii	1980 年文建會的中文資訊交換碼
    euc-tw	Unix 延伸字符集, 包含 CNS11643 平面 1-7
    big5plus	中文數位化技術推廣基金會的 Big5+
    big5ext	中文數位化技術推廣基金會的 Big5e

另外, Encode::HanConvert 模組則提供了簡繁轉換用的兩種編碼:

    big5-simp	Big5 正體中文與 Unicode 簡體中文互轉
    gbk-trad	GBK 簡體中文與 Unicode 正體中文互轉

若想在 GBK 與 Big5 之間互轉, 請參考該模組內附的 b2g.pl 與 g2b.pl 兩支程式,
或在程式內使用下列寫法:

    use Encode::HanConvert;
    $euc_cn = big5_to_gb($big5); # 從 Big5 轉為 GBK
    $big5 = gb_to_big5($euc_cn); # 從 GBK 轉為 Big5

=head2 進一步的資訊

請參考 Perl 內附的大量說明文件 (不幸全是用英文寫的), 來學習更多關於
Perl 的知識, 以及 Unicode 的使用方式. 不過, 外部的資源相當豐富:

=head2 提供 Perl 資源的網址

=over 4

=item L<https://www.perl.org/>

Perl 的首頁

=item L<https://www.perl.com/>

由 Perl 基金會所營運的文章輯錄

=item L<https://www.cpan.org/>

Perl 綜合典藏網 (Comprehensive Perl Archive Network)

=item L<https://lists.perl.org/>

Perl 郵遞論壇一覽

=back

=head2 學習 Perl 的網址

=over 4

=item L<http://www.oreilly.com.cn/index.php?func=booklist&cat=68>

正體中文版的歐萊禮 Perl 書藉

=back

=head2 Perl 使用者集會

=over 4

=item L<https://www.pm.org/groups/taiwan.html>

臺灣 Perl 推廣組一覽

=item L<irc://chat.freenode.org/#perl.tw>

Perl.tw 線上聊天室

=back

=head2 Unicode 相關網址

=over 4

=item L<https://www.unicode.org/>

Unicode 學術學會 (Unicode 標準的制定者)

=item L<http://www.cl.cam.ac.uk/%7Emgk25/unicode.html>

Unix/Linux 上的 UTF-8 及 Unicode 答客問

=back

=head2 中文化資訊

=over 4

=item 中文化軟體聯盟

L<http://www.cpatch.org/>

=back

=head1 SEE ALSO

L<Encode>, L<Encode::TW>, L<perluniintro>, L<perlunicode>

=head1 AUTHORS

Jarkko Hietaniemi E<lt>jhi@iki.fiE<gt>

Audrey Tang (唐鳳) E<lt>audreyt@audreyt.orgE<gt>

=cut
