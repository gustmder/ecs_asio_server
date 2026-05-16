#pragma once

// 모든 프로토콜을 한 번에 포함하는 헤더 파일
// 서버-클라이언트 공유용

#include "protocol_base.hpp"
#include "protocol_player.hpp"
#include "protocol_chat.hpp"
#include "protocol_guild.hpp"
#include "protocol_map.hpp"
#include "protocol_combat.hpp"

namespace lemondory::shared {

// 모든 프로토콜이 포함된 네임스페이스
// 이 파일을 include하면 모든 메시지 타입을 사용할 수 있습니다.

} // namespace lemondory::shared

