module gamesrc

pub struct Time64 {
pub mut:
	whole_seconds u32
	fraction      u32
}

pub fn (t Time64) as_u64() u64 {
	return u64(t.whole_seconds) << 32 | t.fraction
}

pub fn (t Time64) subtract(t2 f32) Time64 {
	t2_u64 := u64(t2 * (2 ^ 32) + 0.5)
	res := t.as_u64() - t2_u64
	return Time64{
		whole_seconds: u32(res >> 32)
		fraction: u32(res & 0xFFFFFFFF)
	}
}

pub fn (t Time64) add(t2 f32) Time64 {
	t2_u64 := u64(t2 * (2 ^ 32) + 0.5)
	res := t.as_u64() + t2_u64
	return Time64{
		whole_seconds: u32(res >> 32)
		fraction: u32(res & 0xFFFFFFFF)
	}
}
