library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity vga_ctrl is
port(
	clk_vga : in std_logic;
	rst_n : in std_logic;
	vram_data : in std_logic_vector(15 downto 0);
	vram_addr : out std_logic_vector(7 downto 0);
	next_line : out std_logic;
	vga_hs : out std_logic;
	vga_vs : out std_logic;
	vga_r : out std_logic_vector(3 downto 0);
	vga_g : out std_logic_vector(3 downto 0);
	vga_b : out std_logic_vector(3 downto 0)
);
end entity;

architecture rtl of vga_ctrl is

signal v_count : std_logic_vector(9 downto 0);
signal h_count : std_logic_vector(9 downto 0);
type t_vst is (VSYNC, VBP, VACTIVE, VFP);
type t_hst is (HSYNC, HBP, HACTIVE, HFP);
signal vst : t_vst;
signal hst : t_hst;

constant VSYNC_LINE : integer := 2 - 1;
constant VBP_LINE : integer := 33 + 24 + VSYNC_LINE;
constant VACTIVE_LINE : integer := 480 - 48 + VBP_LINE; 
constant VFP_LINE : integer := 10 + 24 + VACTIVE_LINE;
constant HSYNC_PIX : integer := 96 - 1;
constant HBP_PIX : integer := 48 + 80 + HSYNC_PIX;
constant HACTIVE_PIX : integer := 640 - 160 + HBP_PIX;
constant HFP_PIX : integer := 16 + 80 + HACTIVE_PIX; 

signal vga_hs_next : std_logic;
signal vga_vs_next : std_logic;
signal vga_r_next : std_logic_vector(3 downto 0);
signal vga_g_next : std_logic_vector(3 downto 0);
signal vga_b_next : std_logic_vector(3 downto 0);

signal vram_addr_r : std_logic_vector(7 downto 0);
type t_vramst is (IDLE, PIX0, PIX1, PIX2);
signal vram_st : t_vramst;

type t_nextlinest is(IDLE, LINE0, LINE1, LINE2);
signal nextlinest : t_nextlinest;

begin

XREG_ST : process(clk_vga, rst_n)
begin
	if rst_n = '0' then
		v_count <= (others=>'0');
		h_count <= (others=>'0');
		vst <= VSYNC;
		hst <= HSYNC;
		vga_hs <= '1';
		vga_vs <= '1';
		vga_r <= (others=>'0');
		vga_g <= (others=>'0');
		vga_b <= (others=>'0');
	elsif clk_vga'event and clk_vga = '1' then
	
		vga_hs <= vga_hs_next;
		vga_vs <= vga_vs_next;
		vga_r <= vga_r_next;
		vga_g <= vga_g_next;
		vga_b <= vga_b_next;
		
		case hst is
		when HSYNC =>
			if unsigned(h_count) = HSYNC_PIX then
				hst <= HBP;
			end if;
		when HBP =>
			if unsigned(h_count) = HBP_PIX then
				hst <= HACTIVE;
			end if;
		when HACTIVE =>
			if unsigned(h_count) = HACTIVE_PIX then
				hst <= HFP;
			end if;
		when HFP =>
			if unsigned(h_count) = HFP_PIX then
				hst <= HSYNC;
			end if;
		end case;
		
		if unsigned(h_count) < HFP_PIX then
			h_count <= h_count + 1;
		else
			h_count <= (others=>'0');
			case vst is
			when VSYNC =>
				if unsigned(v_count) = VSYNC_LINE then
					vst <= VBP;
				end if;
			when VBP =>
				if unsigned(v_count) = VBP_LINE then
					vst <= VACTIVE;
				end if;
			when VACTIVE =>
				if unsigned(v_count) = VACTIVE_LINE then
					vst <= VFP;
				end if;
			when VFP =>
				if unsigned(v_count) = VFP_LINE then
					vst <= VSYNC;
				end if;
			end case;
			
			if unsigned(v_count) < VFP_LINE then
				v_count <= v_count + 1;
			else
				v_count <= (others=>'0');
			end if;
		end if;
	end if;
end process;

vga_hs_next <= '0' when hst = HSYNC else '1';
vga_vs_next <= '0' when vst = VSYNC else '1';

X_VGA : process(vst, hst, h_count, v_count, vram_data)
begin
	if vst = VACTIVE and hst = HACTIVE then
		vga_r_next <= vram_data(14 downto 11);
		vga_g_next <= vram_data(9 downto 6);
		vga_b_next <= vram_data(4 downto 1);
	else
		vga_r_next <= (others=>'0');
		vga_g_next <= (others=>'0');
		vga_b_next <= (others=>'0');
	end if;
end process;

XREG_VRAMST : process(clk_vga, rst_n)
begin
	if rst_n = '0' then
		vram_st <= IDLE;
		vram_addr_r <= (others=>'0');
	elsif clk_vga'event and clk_vga = '1' then
		case vram_st is
		when IDLE =>
			if unsigned(h_count) = HBP_PIX - 5 then
				vram_st <= PIX0;
				vram_addr_r <= (others=>'0');
			end if;
		when PIX0 =>
			vram_st <= PIX1;
		when PIX1 =>
			vram_st <= PIX2;
		when PIX2 =>
			vram_addr_r <= vram_addr_r + 1;
			if vram_addr_r < 159 then
				vram_st <= PIX0;
			else
				vram_st <= IDLE;
			end if;
		end case;
	end if;
end process;

vram_addr <= vram_addr_r;

XREG_NEXTLINE : process(clk_vga, rst_n)
begin
	if rst_n = '0' then
		next_line <= '0';
		nextlinest <= IDLE;
	elsif clk_vga'event and clk_vga = '1' then
		next_line <= '0';
		case nextlinest is
		when IDLE =>
			if unsigned(v_count) = VBP_LINE and unsigned(h_count) = HBP_PIX + 480 then
				nextlinest <= LINE0;
				next_line <= '1';
			end if;
		when LINE0 =>
			if unsigned(h_count) = HBP_PIX + 480 then
				nextlinest <= LINE1;
			end if;
		when LINE1 =>
			if unsigned(h_count) = HBP_PIX + 480 then
				nextlinest <= LINE2;
			end if;
		when LINE2 =>
			if unsigned(h_count) = HBP_PIX + 480 then
				if unsigned(v_count) < VACTIVE_LINE + 1 then
					nextlinest <= LINE0;
					next_line <= '1';
				else
					nextlinest <= IDLE;
				end if;
			end if;
		end case;
	end if;
end process;

end architecture;