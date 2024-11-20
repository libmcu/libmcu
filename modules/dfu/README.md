### 이미지 헤더 생성 방법
```sh
python generate_dfu_header.py <file_path> <dfu_type> [vector_addr]
```

`<file_path>`는 이미지 파일이며, `<dfu_type>`는 `app`, `loader`, `updator` 중 하나입니다. `[vector_addr]`는 선택적인 인자로 벡터 테이블 주소입니다.

예를 들어, `app_image.bin` 헤더를 생성할 경우 `dfu_type`을 `DFU_TYPE_APP`, `vector_addr`를 0x08000000으로 설정하려면 다음과 같이 실행합니다.

```sh
python generate_dfu_header.py app_image.bin DFU_TYPE_APP 0x08000000
```

`vector_addr` 인자를 생략하면 기본값 0이 사용됩니다.

```sh
python generate_dfu_header.py app_image.bin DFU_TYPE_APP
```

### 이미지 헤더를 바이너리 파일 앞에 붙이기
이제 `dfu_image_header.bin` 파일과 이미지 바이너리 파일을 결합합니다. 예를 들어, 이미지 바이너리 파일이 `image.bin`이라고 가정하면, 다음 명령어를 사용하여 결합할 수 있습니다.

```sh
cat dfu_image_header.bin image.bin > combined_image.bin
```

이 명령어는 `dfu_image_header.bin` 파일과 `image.bin` 파일을 결합하여 `combined_image.bin` 파일로 저장합니다. `combined_image.bin` 파일은 구조체 데이터가 앞에 붙은 이미지 바이너리 파일입니다.
