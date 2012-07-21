library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

-- CONTROL register
--   0 : Operation :4
--     0 := NOP
--     1 := GET DO
--     2 := SET CLK LOW
--     3 := SET CLK HIGH
--     4 := SET CS LOW
--     5 := SET CS HIGH
--     6 := SET DI LOW
--     7 := SET DI HIGH
--     8 := SEND DUMMY BYTE
--     9 := WRITE BYTE
--     A := READ BYTE
--   8 : Byte to send :8

-- STATUS register
--   0 : Read Byte 	: 8
--   8 : DO 		: 1

entity av_sdc_config is
port(
	clk : in std_logic;
	rst_n : in std_logic;
	read : in std_logic;
	write : in std_logic;
	waitrequest : out std_logic;
	readdata : out std_logic_vector(31 downto 0);
	writedata : in std_logic_vector(31 downto 0);
	sd_clk : out std_logic;
	sd_cs : out std_logic;
	sd_di : out std_logic;
	sd_do : in std_logic
);
end entity;

architecture rtl of av_sdc_config is

type t_state is (IDLE, READ_REQ, SEND_DUMMY, WRITE_BYTE, READ_BYTE);
signal state_r : t_state;

signal cycle_r : std_logic_vector(4 downto 0);

signal byte_r : std_logic_vector(7 downto 0);

begin

XREG_ST : process(clk, rst_n)
begin
	if rst_n = '0' then
		readdata <= (others=>'0');
		sd_cs <= '0';
		sd_di <= '0';
		sd_clk <= '0';
		state_r <= IDLE;
		cycle_r <= (others=>'0');
		byte_r <= (others=>'0');
	elsif clk'event and clk = '1' then
		readdata <= X"0000" & "0000000" & sd_do & byte_r;
		
		case state_r is
		when IDLE =>
			if write = '1' then
				case writedata(3 downto 0) is
				when X"2" =>	-- SET CLK LOW
					sd_clk <= '0';
				when X"3" =>	-- SET CLK HIGH
					sd_clk <= '1';
				when X"4" =>	-- SET CS LOW
					sd_cs  <= '0';
				when X"5" =>	-- SET CS HIGH
					sd_cs  <= '1';
				when X"6" =>	-- SET DI LOW
					sd_di  <= '0';
				when X"7" =>	-- SET DI HIGH
					sd_di  <= '1';
				when X"8" =>    -- SEND DUMMY BYTE
					cycle_r <= (others=>'0');
					state_r <= SEND_DUMMY;
				when X"9" =>    -- WRITE BYTE
					cycle_r <= (others=>'0');
					state_r <= WRITE_BYTE;
					byte_r <= writedata(15 downto 8);
				when X"A" =>    -- READ BYTE
					cycle_r <= (others=>'0');
					state_r <= READ_BYTE;
					byte_r <= (others=>'0');
				when others =>
				end case;
			elsif read = '1' then
				state_r <= READ_REQ;
			end if;
		when READ_REQ =>
			state_r <= IDLE;
		when SEND_DUMMY =>
			cycle_r <= cycle_r + 1;
			if cycle_r = "11111" then
				state_r <= IDLE;
			end if;
			
			sd_clk <= cycle_r(1) xor cycle_r(0);
		when WRITE_BYTE =>
			cycle_r <= cycle_r + 1;
			if cycle_r = "11111" then
				state_r <= IDLE;
			end if;
			
			sd_clk <= cycle_r(1) xor cycle_r(0);
			
			if cycle_r(1 downto 0) = "00" then
				sd_di <= byte_r(7);
				byte_r <= byte_r(6 downto 0) & "0";
			end if;
		when READ_BYTE =>
			cycle_r <= cycle_r + 1;
			if cycle_r = "11111" then
				state_r <= IDLE;
			end if;
			
			sd_clk <= cycle_r(1) xor cycle_r(0);
			
			if cycle_r(1 downto 0) = "01" then
				byte_r <= byte_r(6 downto 0) & sd_do;
			end if;
		end case;
	end if;
end process;

XWAIT : process(state_r, read)
begin
	case state_r is
	when IDLE =>
		if read = '1' then
			waitrequest <= '1';
		else
			waitrequest <= '0';
		end if;
	when READ_REQ =>
		waitrequest <= '0';
	when others =>
		waitrequest <= '1';
	end case;

end process;

end architecture;