# XLibConcurrentCrossword

WSPÓLNE ROZWIĄZYWANIE KRZYŻÓWKI

Program powinien umożliwiać rozwiązywanie krzyżówki kilku użytkownikom
jednocześnie (użytkownicy mają prawo przyłączać się do zabawy w dowolnym
momencie czasu i w dowolnym momencie porzucać ją, niezależnie od siebie).

Pierwszy przyłączony użytkownik powinien zobaczyć wyświetloną przez
program pustą krzyżówkę, a obok niej opisy haseł wraz z numerami
początków (kilka haseł poziomo i kilka haseł pionowo). Użytkownicy
mogą odgadywać i wpisywać hasła w dowolnej kolejności. Po wpisaniu
kompletnego hasła powinno nastąpić jego zatwierdzenie przez wpisującego -
program wtedy sprawdza, czy hasło zostało odgadnięte prawidłowo, i jeśli
tak, to czyni je widocznym dla wszystkich aktualnych użytkowników,
a jeśli nie, to wymazuje je i powiadamia wpisującego o błędzie. Zabawa
kończy się, kiedy krzyżówka jest już w całości wypełniona.

Uwaga: użytkownicy mogą współbieżnie wprowadzać hasła, które w krzyżówce
       nie przecinają się. Jeśli przecinają się, należy zwrócić uwagę na
       możliwość wystąpienia konfliktu (i jakoś to rozwiązać).
